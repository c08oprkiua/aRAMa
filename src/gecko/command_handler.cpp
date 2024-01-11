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

#define CHECK_ERROR(cond)     \
	if (cond)                 \
	{                         \
		line = __LINE__; \
		goto error;       \
	}

#define errno2 (*__gh_errno_ptr())
#define MSG_DONT_WAIT 32
#define E_WOULD_BLOCK 6
#define WRITE_SCREEN_MESSAGE_BUFFER_SIZE 100
#define SERVER_VERSION "06/03/2017"

#define ONLY_ZEROS_READ 0xB0
#define NON_ZEROS_READ 0xBD
#define VERSION_HASH 0x7FB223

#define FS_MAX_LOCALPATH_SIZE           511
#define FS_MAX_MOUNTPATH_SIZE           128
#define FS_MAX_FULLPATH_SIZE            (FS_MAX_LOCALPATH_SIZE + FS_MAX_MOUNTPATH_SIZE)

#define DISASSEMBLER_BUFFER_SIZE 0x1024

// The time the producer and consumer wait while there is nothing to do
#define WAITING_TIME_MILLISECONDS 1


void CommandHandler::command_write_8()
{
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0);

	char *destinationAddress = ((char **)buffer)[0];
	*destinationAddress = buffer[7];
	DCFlushRange(destinationAddress, 1);
	return;
};

void CommandHandler::command_write_16()
{
	short *destinationAddress;
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	destinationAddress = ((short **)buffer)[0];
	*destinationAddress = ((short *)buffer)[3];
	DCFlushRange(destinationAddress, 2);
	return;
};

void CommandHandler::command_write_32()
{
	int destinationAddress, value;
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	destinationAddress = ((int *)buffer)[0];
	value = ((int *)buffer)[1];

	GeckoKernelCopyData((unsigned char *) destinationAddress, (unsigned char *) &value, sizeof(int));
	
};

void CommandHandler::command_validate_address_range(){
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	// Retrieve the data
	int startingAddress = ((int *)buffer)[0];
	int endingAddress = ((int *)buffer)[1];

	int isAddressRangeValid = validateAddressRange(startingAddress, endingAddress);

	sendByte((unsigned char)isAddressRangeValid);
};

void CommandHandler::command_disassemble_range(){
	// Receive the starting, ending address and the disassembler options
	ret = recvwait( 4 + 4 + 4);
	CHECK_ERROR(ret < 0)
	void *startingAddress = ((void **)buffer)[0];
	void *endingAddress = ((void **)buffer)[1];
	int disassemblerOptions = ((int *)buffer)[2];

	// Disassemble
	DisassemblePPCRange(startingAddress, endingAddress, formatDisassembled, OSGetSymbolName,
						(DisassemblePPCFlags)disassemblerOptions);

	// Send the disassembler buffer size
	int length = DISASSEMBLER_BUFFER_SIZE;
	ret = sendwait_buffer(&length, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (disassembler buffer size)")

	// Send the data
	ret = sendwait_buffer(disassemblerBufferPointer, length);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (disassembler buffer)")

	// Place the pointer back to the beginning
	disassemblerBuffer = (char *)disassemblerBufferPointer;
}

void CommandHandler::command_memory_disassemble(){
	// Receive the starting address, ending address and disassembler options
	ret = recvwait(sizeof(int) * 3);
	CHECK_ERROR(ret < 0)
	int startingAddress = ((int *)buffer)[0];
	int endingAddress = ((int *)buffer)[1];
	int disassemblerOptions = ((int *)buffer)[2];

	uint32_t currentAddress = startingAddress;
	int bufferSize = PPC_DISASM_MAX_BUFFER;
	int integerSize = 4;

	// Disassemble everything
	while (currentAddress < endingAddress)
	{
		int currentIntegerIndex = 0;

		while ((currentIntegerIndex < (DATA_BUFFER_SIZE / integerSize)) && (currentAddress < endingAddress))
		{
			int value = *(int *)currentAddress;
			((int *)buffer)[currentIntegerIndex++] = value;
			char *opCodeBuffer = (char *)malloc(bufferSize);
			bool status = (bool)DisassemblePPCOpcode(&currentAddress, opCodeBuffer, bufferSize,
													 OSGetSymbolName,
													 (DisassemblePPCFlags)disassemblerOptions);

			((int *)buffer)[currentIntegerIndex++] = status;

			if (status == 1)
			{
				// Send the length of the opCode buffer string
				int length = strlen(opCodeBuffer);
				((int *)buffer)[currentIntegerIndex++] = length;

				// Send the opCode buffer itself
				memcpy(buffer + (currentIntegerIndex * integerSize), opCodeBuffer, length);
				currentIntegerIndex += (roundUpToAligned(length) / integerSize);
			}

			free(opCodeBuffer);
			currentAddress += integerSize;
		}

		int bytesToSend = currentIntegerIndex * integerSize;
		ret = sendwait_buffer((unsigned char *)&bytesToSend, sizeof(int));
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Buffer size)");

		// VALUE(4)|STATUS(4)|LENGTH(4)|DISASSEMBLED(LENGTH)
		ret = sendwait(bytesToSend);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Buffer)");
	}

	int bytesToSend = 0;
	ret = sendwait_buffer((unsigned char *)&bytesToSend, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (No more bytes)")
};

void CommandHandler::command_read_memory_compressed(){
	// Receive the starting address and length 
/*	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	int startingAddress = ((int *)buffer)[0];
	unsigned int inputLength = ((unsigned int *)buffer)[1];

	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	// Initialize the stream struct
	ret = deflateInit(&stream, Z_BEST_COMPRESSION);
	ASSERT_INTEGER(ret, Z_OK, "deflateInit")

	// Supply the data
	stream.avail_in = inputLength;
	stream.next_in = (Bytef *)startingAddress;
	stream.avail_out = DATA_BUFFER_SIZE;
	void *outputBuffer = (void *)(&buffer + 4);
	stream.next_out = (Bytef *)outputBuffer;

	// Deflate
	ret = deflate(&stream, Z_FINISH);
	ASSERT_INTEGER(ret, Z_OK, "deflate");

	// Finish
	ret = deflateEnd(&stream);
	ASSERT_INTEGER(ret, Z_OK, "deflateEnd");

	// Send the compressed buffer size and content
	int deflatedSize = stream.total_out;
	((int *)buffer)[0] = deflatedSize;
	ret = sendwait(bss, clientfd, buffer, 4 + deflatedSize);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Compressed data)")

	break;
	* /

	// https://www.gamedev.net/resources/_/technical/game-programming/in-memory-data-compression-and-decompression-r2279
	/*

	// Setup compressed buffer
	unsigned int compressedBufferSize = length * 2;
	void *compressedBuffer = (void *) OSAllocFromSystem(compressedBufferSize, 0x4);
	ASSERT_ALLOCATED(compressedBuffer, "Compressed buffer")

	unsigned int zlib_handle;
	OSDynLoad_Acquire("zlib125.rpl", (uint32_t *) &zlib_handle);
	int (*compress2)(char *, int *, const char *, int, int);
	OSDynLoad_FindExport((uint32_t) zlib_handle, 0, "compress2", &compress2);

	int destinationBufferSize;
	int status = compress2((char *) compressedBuffer, &destinationBufferSize,
						   (const char *) rawBuffer, length, Z_DEFAULT_COMPRESSION);

	ret = sendwait(bss, clientfd, &status, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (status)")

	if (status == Z_OK) {
		// Send the compressed buffer size and content
		((int *) buffer)[0] = destinationBufferSize;
		memcpy(buffer + 4, compressedBuffer, destinationBufferSize);

		ret = sendwait(bss, clientfd, buffer, 4 + destinationBufferSize);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Compressed data)")
	}

	free(rawBuffer);
	OSFreeToSystem(compressedBuffer);

	break;*/
};

void CommandHandler::command_kernel_write(){
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	void *address = ((void **)buffer)[0];
	void *value = ((void **)buffer)[1];

	writeKernelMemory(address, (uint32_t)value);
};

void CommandHandler::command_kernel_read(){
	ret = recvwait(sizeof(int));
	CHECK_ERROR(ret < 0)

	void *address = ((void **)buffer)[0];
	void *value = (void *)readKernelMemory(address);

	*(void **)buffer = value;
	sendwait(sizeof(int));
};

void CommandHandler::command_take_screenshot(){
	// Tell the hook to dump the screen shot now
	shouldTakeScreenShot = true;

	// Tell the client the size of the upcoming image
	ret = sendwait_buffer((unsigned char *)&totalImageSize, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (total image size)")

	// Keep sending the image data
	while (remainingImageSize > 0)
	{
		int bufferPosition = 0;

		// Fill the buffer till it is full
		while (bufferPosition <= DATA_BUFFER_SIZE)
		{
			// Wait for data to be available
			while (bufferedImageSize == 0)
			{
				usleep(WAITING_TIME_MILLISECONDS);
			}

			memcpy(buffer + bufferPosition, bufferedImageData, bufferedImageSize);
			bufferPosition += bufferedImageSize;
			bufferedImageSize = 0;
		}

		// Send the size of the current chunk
		ret = sendwait_buffer((unsigned char *)&bufferPosition, sizeof(int));
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image data chunk size)")

		// Send the image data itself
		ret = sendwait(bufferPosition);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image data)")
	}

	/*GX2ColorBuffer colorBuffer;
	// TODO Initialize colorBuffer!
	GX2Surface surface = colorBuffer.surface;
	void *image_data = surface.image_data;
	uint32_t image_size = surface.image_size;

	// Send the image size so that the client knows how much to read
	ret = sendwait(bss, clientfd, (unsigned char *) &image_size, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image size)")

	unsigned int imageBytesSent = 0;
	while (imageBytesSent < image_size) {
		int length = image_size - imageBytesSent;

		// Do not smash the buffer
		if (length > DATA_BUFFER_SIZE) {
			length = DATA_BUFFER_SIZE;
		}

		// Send the image bytes
		memcpy(buffer, image_data, length);
		ret = sendwait(bss, clientfd, buffer, length);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image bytes)")

		imageBytesSent += length;
	}*/
};

void CommandHandler::command_upload_memory(){
	// Receive the starting and ending addresses
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	unsigned char *currentAddress = ((unsigned char **)buffer)[0];
	unsigned char *endAddress = ((unsigned char **)buffer)[1];

	while (currentAddress != endAddress)
	{
		int length;

		length = (int)(endAddress - currentAddress);
		if (length > DATA_BUFFER_SIZE)
		{
			length = DATA_BUFFER_SIZE;
		}

		ret = recvwait(length);
		CHECK_ERROR(ret < 0)
		GeckoKernelCopyData(currentAddress, buffer, (unsigned int) length);

		currentAddress += length;
	}
};

void CommandHandler::command_server_status()
{
	ret = sendByte(1);
	CHECK_ERROR(ret < 0)
	return;
};

void CommandHandler::command_server_version(){
	char versionBuffer[50];
	strcpy(versionBuffer, SERVER_VERSION);
	int versionLength = strlen(versionBuffer);
	((int *)buffer)[0] = versionLength;
	memcpy(buffer + sizeof(int), versionBuffer, versionLength);

	// Send the length and the version string
	ret = sendwait(sizeof(int) + versionLength);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (server version)");
};

void CommandHandler::command_get_data_buffer_size(){
	log_printf("COMMAND_GET_DATA_BUFFER_SIZE...\n");
	((int *)buffer)[0] = DATA_BUFFER_SIZE;
	log_printf("Sending buffer size...\n");
	ret = sendwait(sizeof(int));
	log_printf("Sent: %i\n", ret);
	CHECK_ERROR(ret < 0)
};

void CommandHandler::command_get_code_handler_address(){
	((int *)buffer)[0] = CODE_HANDLER_INSTALL_ADDRESS;
	ret = sendwait(4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (code handler address)")
} // Depreciate?

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

void CommandHandler::command_account_identifier(){
	// Acquire the RPL

	/*Unneeded in WUT

	uint32_t nn_act_handle;
	OSDynLoad_Acquire("nn_act.rpl", &nn_act_handle);

	// Acquire the functions via their mangled file names
	int (*nn_act_Initialize)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "Initialize__Q2_2nn3actFv", &nn_act_Initialize);
	ASSERT_ALLOCATED(nn_act_Initialize, "nn_act_Initialize")
	unsigned char (*nn_act_GetSlotNo)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetSlotNo__Q2_2nn3actFv", &nn_act_GetSlotNo);
	ASSERT_ALLOCATED(nn_act_GetSlotNo, "nn_act_GetSlotNo")
	unsigned int (*nn_act_GetPersistentIdEx)(unsigned char);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetPersistentIdEx__Q2_2nn3actFUc", &nn_act_GetPersistentIdEx);
	ASSERT_ALLOCATED(nn_act_GetPersistentIdEx, "nn_act_GetPersistentIdEx")
	int (*nn_act_Finalize)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "Finalize__Q2_2nn3actFv", &nn_act_Finalize);
	ASSERT_ALLOCATED(nn_act_Finalize, "nn_act_Finalize")
	*/

	// Get the identifier
	// ret = nn_act_Initialize();
	ret = (int) nn::act::Initialize();
	// ASSERT_INTEGER(ret, 1, "Initializing account library");

	nn::act::SlotNo slotNumber = nn::act::GetSlotNo();
	nn::act::PersistentId persistentIdentifier = nn::act::GetPersistentIdEx(slotNumber);
	ret = (int) nn::act::Finalize();
	ASSERT_FUNCTION_SUCCEEDED(ret, "nn_act_Finalize");

	// Send it
	ret = sendwait_buffer((unsigned char *)&persistentIdentifier, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (persistent identifier)")
};

void CommandHandler::command_write_screen(){/*
	char message[WRITE_SCREEN_MESSAGE_BUFFER_SIZE];
	ret = recvwait(4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (write screen seconds)")
	int seconds = ((int *)buffer)[0];
	receiveString(bss, clientfd, message, WRITE_SCREEN_MESSAGE_BUFFER_SIZE);
	writeScreen(message, seconds);

	break;*/
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

void CommandHandler::command_memory_search_32(){
	ret = recvwait(sizeof(int) * 3);
	CHECK_ERROR(ret < 0);
	int address = ((int *)buffer)[0];
	int value = ((int *)buffer)[1];
	int length = ((int *)buffer)[2];
	int index;
	int foundAddress = 0;
	for (index = address; index < address + length; index += sizeof(int))
	{
		if (*(int *)index == value)
		{
			foundAddress = index;
			break;
		}
	}

	((int *)buffer)[0] = foundAddress;
	ret = sendwait(sizeof(int));
	CHECK_ERROR(ret < 0)
};

void CommandHandler::command_advanced_memory_search(){
	// Receive the initial data
	ret = recvwait(6 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (memory search information)")
	int bufferIndex = 0;
	int startingAddress = ((int *)buffer)[bufferIndex++];
	int length = ((int *)buffer)[bufferIndex++];
	int kernelRead = ((int *)buffer)[bufferIndex++];
	int resultsLimit = ((int *)buffer)[bufferIndex++];
	int aligned = ((int *)buffer)[bufferIndex++];
	int searchBytesCount = ((int *)buffer)[bufferIndex];

	// Receive the search bytes
	char searchBytes[searchBytesCount];
	ret = recvwait_buffer((unsigned char *)searchBytes, searchBytesCount);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (memory search bytes)")

	int iterationIncrement = aligned ? searchBytesCount : 1;
	int searchBytesOccurrences = 0;

	// Perform the bytes search and collect the results
	for (int currentAddress = startingAddress;
		 currentAddress < startingAddress + length;
		 currentAddress += iterationIncrement)
	{

		int comparisonResult;

		if (kernelRead)
		{
			comparisonResult = kernelMemoryCompare((char *)currentAddress,
												   searchBytes, searchBytesCount);
		}
		else
		{
			comparisonResult = memoryCompare((void *)currentAddress,
											 searchBytes, (size_t)searchBytesCount);
		}
		if (comparisonResult == 0)
		{
			// Search bytes have been found, add the addresses to the return buffer
			((int *)buffer)[1 + searchBytesOccurrences] = currentAddress;
			searchBytesOccurrences++;

			if ((resultsLimit == searchBytesOccurrences) || (searchBytesOccurrences == ((DATA_BUFFER_SIZE / 4) - 1)))
			{
				// We bail out
				break;
			}
		}
	}

	((int *)buffer)[0] = searchBytesOccurrences * 4;
	ret = sendwait(4 + (searchBytesOccurrences * 4));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (Sending search bytes occurrences)")
};

void CommandHandler::command_pause_console(){
	writeConsoleState(PAUSED);
};
void CommandHandler::command_resume_console(){
	writeConsoleState(RUNNING);
};
void CommandHandler::command_is_console_paused(){
	bool paused = isConsolePaused();
	log_printf("Paused: %d\n", paused);
	ret = sendByte((unsigned char)paused);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendByte (sending paused console status)")
};

void CommandHandler::command_get_os_version(){
	MCPSystemVersion version;
	//Idk what to put for handle
	MCP_GetSystemVersion(0, &version);

	((int *)buffer)[0] = version.major;
	((int *)buffer)[1] = version.minor;
	((int *)buffer)[2] = version.patch;
	buffer[3] = version.region;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (OS version)");
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

void CommandHandler::command_run_kernel_copy_service(){
	if (!kernelCopyServiceStarted)
	{
		kernelCopyServiceStarted = true;
		startKernelCopyService();
	}
};

void CommandHandler::command_iosu_hax_read_file(){
	/*log_print("COMMAND_IOSUHAX_READ_FILE");

	// TODO Crashes console on this call
	int returnValue = IOSUHAX_Open(NULL);
	log_print("IOSUHAX_Open Done");
	log_printf("IOSUHAX_Open: %i", returnValue);

	if (returnValue < 0) {
		goto IOSUHAX_OPEN_FAILED;
	}

	int fileSystemFileDescriptor = IOSUHAX_FSA_Open();
	log_printf("IOSUHAX_FSA_Open: %i", fileSystemFileDescriptor);

	if (fileSystemFileDescriptor < 0) {
		goto IOSUHAX_FSA_OPEN_FAILED;
	}

	int fileDescriptor;
	const char *filePath = "/vol/storage_usb/usr/title/0005000e/1010ed00/content/audio/stream/pBGM_GBA_CHEESELAND_F.bfstm";
	returnValue = IOSUHAX_FSA_OpenFile(fileSystemFileDescriptor, filePath, "rb", &fileDescriptor);
	log_printf("IOSUHAX_FSA_OpenFile: %i", returnValue);

	if (returnValue < 0) {
		goto IOSUHAX_OPEN_FILE_FAILED;
	}

	fileStat_s fileStat;
	returnValue = IOSUHAX_FSA_StatFile(fileSystemFileDescriptor, fileDescriptor, &fileStat);
	log_printf("IOSUHAX_FSA_StatFile: %i", returnValue);

	if (returnValue < 0) {
		goto IOSUHAX_READ_FILE_FAILED_CLOSE;
	}

	void *fileBuffer = MEMBucket_alloc(fileStat.size, 4);
	log_printf("File Buffer: %p", fileBuffer);

	if (!fileBuffer) {
		goto IOSUHAX_READ_FILE_FAILED_CLOSE;
	}

	size_t totalBytesRead = 0;
	while (totalBytesRead < fileStat.size) {
		size_t remainingBytes = fileStat.size - totalBytesRead;
		int bytesRead = IOSUHAX_FSA_ReadFile(fileSystemFileDescriptor,
											 fileBuffer + totalBytesRead,
											 0x01,
											 remainingBytes,
											 fileDescriptor,
											 0);
		log_printf("IOSUHAX_FSA_ReadFile: %i", bytesRead);

		if (bytesRead <= 0) {
			goto IOSUHAX_READ_FILE_FAILED_CLOSE;
		} else {
			totalBytesRead += bytesRead;
		}
	}

	log_printf("Bytes read: %i", totalBytesRead);

	IOSUHAX_READ_FILE_FAILED_CLOSE:

	returnValue = IOSUHAX_FSA_CloseFile(fileSystemFileDescriptor, fileDescriptor);
	log_printf("IOSUHAX_FSA_CloseFile: %i", returnValue);

	IOSUHAX_OPEN_FILE_FAILED:

	returnValue = IOSUHAX_FSA_Close(fileSystemFileDescriptor);
	log_printf("IOSUHAX_FSA_Close: %i", returnValue);

	IOSUHAX_FSA_OPEN_FAILED:

	returnValue = IOSUHAX_Close();
	log_printf("IOSUHAX_Close: %i", returnValue);

	IOSUHAX_OPEN_FAILED:*/

};

void CommandHandler::command_get_version_hash(){
	((int *)buffer)[0] = VERSION_HASH;
	ret = sendwait(4);
};