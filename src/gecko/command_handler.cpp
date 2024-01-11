#include "command_handler.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/filesystem.h>
#include <coreinit/thread.h> 
#include <coreinit/memory.h>
#include <coreinit/dynload.h>
#include <coreinit/mcp.h>

#include <nn/act.h>

#include <nsysnet/socket.h> 

#include <gx2/event.h>

#include <kernel/kernel.h>

#include "../arama.h"
#include "../tcpgecko/address.h"
#include "../tcpgecko/assertions.h"
#include "../tcpgecko/disassembler.h"
#include "../tcpgecko/threads.h"
#include "../tcpgecko/kernel.h"

#define errno2 (*__gh_errno_ptr())
#define MSG_DONT_WAIT 32
#define E_WOULD_BLOCK 6
#define WRITE_SCREEN_MESSAGE_BUFFER_SIZE 100


#define ONLY_ZEROS_READ 0xB0
#define NON_ZEROS_READ 0xBD

#define FS_MAX_LOCALPATH_SIZE           511
#define FS_MAX_MOUNTPATH_SIZE           128
#define FS_MAX_FULLPATH_SIZE            (FS_MAX_LOCALPATH_SIZE + FS_MAX_MOUNTPATH_SIZE)

#define DISASSEMBLER_BUFFER_SIZE 0x1024

// The time the producer and consumer wait while there is nothing to do
#define WAITING_TIME_MILLISECONDS 1

void CommandHandler::command_validate_address_range(){
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	// Retrieve the data
	int startingAddress = ((int *)buffer)[0];
	int endingAddress = ((int *)buffer)[1];

	int isAddressRangeValid = validateAddressRange(startingAddress, endingAddress);

	sendByte((unsigned char)isAddressRangeValid);
};



void CommandHandler::command_read_threads(){
	struct node *threads = getAllThreads();
	int threadCount = length(threads);
	log_printf("Thread Count: %i\n", threadCount);

	// Send the thread count
	log_print("Sending thread count...\n");
	((int *)buffer)[0] = threadCount;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (thread count)");

	// Send the thread addresses and data
	struct node *currentThread = threads;
	while (currentThread != NULL)
	{
		int data = (int)currentThread->data;
		log_printf("Thread data: %08x\n", data);
		((int *)buffer)[0] = (int)currentThread->data;
		memcpy(buffer + sizeof(int), currentThread->data, THREAD_SIZE);
		log_print("Sending node...\n");
		ret = sendwait(sizeof(int) + THREAD_SIZE);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (thread address and data)")

		currentThread = currentThread->next;
	}

	destroy(threads);
};

void CommandHandler::command_follow_pointer(){
	ret = recvwait(sizeof(int) * 2);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (Pointer address and offsets count)")

	// Retrieve the pointer address and amount of offsets
	int baseAddress = ((int *)buffer)[0];
	int offsetsCount = ((int *)buffer)[1];

	// Receive the offsets
	ret = recvwait(offsetsCount * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (offsets)")
	int offsets[offsetsCount];
	int offsetIndex = 0;
	for (; offsetIndex < offsetsCount; offsetIndex++)
	{
		offsets[offsetIndex] = ((int *)buffer)[offsetIndex];
	}

	int destinationAddress = baseAddress;

#define INVALID_ADDRESS -1

	if ((bool)OSIsAddressValid(destinationAddress))
	{
		// Apply pointer offsets
		for (offsetIndex = 0; offsetIndex < offsetsCount; offsetIndex++)
		{
			int pointerValue = *(int *)destinationAddress;
			int offset = offsets[offsetIndex];
			destinationAddress = pointerValue + offset;

			// Validate the pointer address
			bool isValidDestinationAddress = (bool)OSIsAddressValid(destinationAddress);

			// Bail out if invalid
			if (!isValidDestinationAddress)
			{
				destinationAddress = INVALID_ADDRESS;

				break;
			}
		}
	}
	else if (offsetsCount > 0)
	{
		// Following pointers failed
		destinationAddress = INVALID_ADDRESS;
	}

	// Return the destination address
	((int *)buffer)[0] = destinationAddress;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (destination address)")
};

void CommandHandler::command_remote_procedure_call(){
	int r3, r4, r5, r6, r7, r8, r9, r10;

	log_print("Receiving RPC information...\n");
	ret = recvwait(sizeof(int) + 8 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "revcwait() Receiving RPC information")
	log_print("RPC information received...\n");

	long long (*function)(int, int, int, int, int, int, int, int);
	function = (long long int (*)(int, int, int, int, int, int, int, int))((void **)buffer)[0];
	r3 = ((int *)buffer)[1];
	r4 = ((int *)buffer)[2];
	r5 = ((int *)buffer)[3];
	r6 = ((int *)buffer)[4];
	r7 = ((int *)buffer)[5];
	r8 = ((int *)buffer)[6];
	r9 = ((int *)buffer)[7];
	r10 = ((int *)buffer)[8];

	log_print("Calling function...\n");
	long long result = function(r3, r4, r5, r6, r7, r8, r9, r10);
	log_printf("Function successfully called with return value: 0x%08x 0x%08x\n", (int)(result >> 32),
			   (int)result);

	log_print("Sending result...\n");
	((long long *)buffer)[0] = result;
	ret = sendwait(sizeof(long long));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait() Sending return value")
	log_print("Result successfully sent...\n");
};

void CommandHandler::command_get_symbol(){
	int size = recvbyte();
	CHECK_ERROR(size < 0)

	ret = recvwait(size);
	CHECK_ERROR(ret < 0)

	/* Identify the RPL name and symbol name */
	char *rplName = (char *)&((int *)buffer)[2];
	char *symbolName = (char *)(&buffer[0] + ((int *)buffer)[1]);

	/* Get the symbol and store it in the buffer */
	OSDynLoad_Module module_handle;
	void *function_address;
	OSDynLoad_Acquire(rplName, &module_handle);

	OSDynLoad_ExportType data = (OSDynLoad_ExportType)recvbyte(bss, clientfd);
	OSDynLoad_FindExport(module_handle, data, symbolName, &function_address);

	((int *)buffer)[0] = (int)function_address;
	ret = sendwait(4);
	CHECK_ERROR(ret < 0)
};

void CommandHandler::command_poke_registers(){
	log_print("Receiving poke registers data...\n");
	int gprSize = 4 * 32;
	int fprSize = 8 * 32;
	ret = recvwait(gprSize + fprSize);
	log_print("Poking registers...\n");
	memcpy((void *)crashContext.gpr, (const void *)buffer, gprSize);
	memcpy((void *)crashContext.fpr, (const void *)buffer, fprSize);
};

void CommandHandler::command_get_stack_trace(){
	log_print("Getting stack trace...\n");
	struct node *stackTrace = getStackTrace(NULL);
	int stackTraceLength = length(stackTrace);

	// Let the client know the length beforehand
	int bufferIndex = 0;
	((int *)buffer)[bufferIndex++] = stackTraceLength;

	struct node *currentStackTraceElement = stackTrace;
	while (currentStackTraceElement != NULL)
	{
		int address = (int)currentStackTraceElement->data;
		log_printf("Stack trace element address: %08x\n", address);
		((int *)buffer)[bufferIndex++] = (int)currentStackTraceElement->data;

		currentStackTraceElement = currentStackTraceElement->next;
	}

	log_printf("Sending stack trace with length %i\n", stackTraceLength);
	ret = sendwait(sizeof(int) + stackTraceLength);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (stack trace)");
};

void CommandHandler::commmand_get_entry_point_address(){
	uint32_t *entryPointAddress = (uint32_t *)*((uint32_t *)OS_SPECIFICS->addr_OSTitle_main_entry);
	((uint32_t *)buffer)[0] = (uint32_t)entryPointAddress;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Entry point address)");
};