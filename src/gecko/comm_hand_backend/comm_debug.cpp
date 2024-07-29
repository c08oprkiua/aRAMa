#include "gecko/command_handler.h"
#include "core/node_list_plus_plus.h"
#include <coreinit/dynload.h>


#define INVALID_ADDRESS -1

NodeList<int> *getStackTrace(OSContext *context);
bool isValidStackPointer(uint32_t stackPointer);

void CommandHandler::command_validate_address_range(){
	ret = recvwait(sizeof(int) * 2);
	//CHECK_ERROR(ret < 0);

	// Retrieve the data
	int startingAddress = ((int *)buffer)[0];
	int endingAddress = ((int *)buffer)[1];

	int isAddressRangeValid = __OSValidateAddressSpaceRange(1, startingAddress, endingAddress - startingAddress + 1);

	sendByte((uint8_t)isAddressRangeValid);
	return;
	
	error:
	error = ret;
	return;
};

void CommandHandler::command_follow_pointer(){
	ret = recvwait(sizeof(int) * 2);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (Pointer address and offsets count)")

	// Retrieve the pointer address and amount of offsets
	int baseAddress = ((int *)buffer)[0];
	int offsetsCount = ((int *)buffer)[1];

	// Receive the offsets
	ret = recvwait(offsetsCount * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (offsets)");
	int offsets[offsetsCount];
	int offsetIndex = 0;
	for (; offsetIndex < offsetsCount; offsetIndex++)
	{
		offsets[offsetIndex] = ((int *)buffer)[offsetIndex];
	}

	int destinationAddress = baseAddress;

	if (OSIsAddressValid(destinationAddress)){
		// Apply pointer offsets
		for (offsetIndex = 0; offsetIndex < offsetsCount; offsetIndex++)
		{
			int pointerValue = *(int *)destinationAddress;
			int offset = offsets[offsetIndex];
			destinationAddress = pointerValue + offset;

			// Validate the pointer address
			bool isValidDestinationAddress = OSIsAddressValid(destinationAddress);

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
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (destination address)");
};

void CommandHandler::command_remote_procedure_call(){
	int r3, r4, r5, r6, r7, r8, r9, r10;

	WHBLogPrint("Receiving RPC information...\n");
	ret = recvwait(sizeof(int) + 8 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "revcwait() Receiving RPC information");
	WHBLogPrint("RPC information received...\n");

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

	WHBLogPrint("Calling function...\n");
	long long result = function(r3, r4, r5, r6, r7, r8, r9, r10);
	WHBLogPrintf("Function successfully called with return value: 0x%08x 0x%08x\n", (int)(result >> 32),
			   (int)result);

	WHBLogPrint("Sending result...\n");
	((long long *)buffer)[0] = result;
	ret = sendwait(sizeof(long long));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait() Sending return value");
	WHBLogPrint("Result successfully sent...\n");
};

void CommandHandler::command_get_symbol(){
	int size = recvbyte();
	char *rplName;
	char *symbolName;
	OSDynLoad_ExportType data;

	CHECK_ERROR(size < 0);

	ret = recvwait(size);
	CHECK_ERROR(ret < 0);

	/* Identify the RPL name and symbol name */
	rplName = (char *)&((int *)buffer)[2];
	symbolName = (char *)(&buffer[0] + ((int *)buffer)[1]);

	/* Get the symbol and store it in the buffer */
	OSDynLoad_Module module_handle;
	void *function_address;
	OSDynLoad_Acquire(rplName, &module_handle);

	data = (OSDynLoad_ExportType) recvbyte();
	OSDynLoad_FindExport(module_handle, data, symbolName, &function_address);

	((int *)buffer)[0] = (int)function_address;
	ret = sendwait(4);
	CHECK_ERROR(ret < 0);

	error:
	error = ret;
	return;
};

//Yes this is not done, ik.
OSContext crashContext;

void CommandHandler::command_poke_registers(){
	WHBLogPrint("Receiving poke registers data...\n");
	int gprSize = 4 * 32;
	int fprSize = 8 * 32;
	ret = recvwait(gprSize + fprSize);
	WHBLogPrint("Poking registers...\n");
	memcpy((void *)crashContext.gpr, (const void *)buffer, gprSize);
	memcpy((void *)crashContext.fpr, (const void *)buffer, fprSize);
};

void CommandHandler::command_get_stack_trace(){
	WHBLogPrint("Getting stack trace...\n");
	
	NodeList<int> *stackTrace = getStackTrace(NULL);
	int stackTraceLength = stackTrace->length();

	// Let the client know the length beforehand
	int bufferIndex = 0;
	((int *)buffer)[bufferIndex++] = stackTraceLength;

	Node<int> currentStackTraceElement = *stackTrace->get_list();
	//while (currentStackTraceElement != nullptr){
	while (currentStackTraceElement.data != NULL){
		int address = currentStackTraceElement.data;
		WHBLogPrintf("Stack trace element address: %08x\n", address);
		((int *)buffer)[bufferIndex++] = currentStackTraceElement.data;

		currentStackTraceElement = *currentStackTraceElement.next;
	}

	WHBLogPrintf("Sending stack trace with length %i\n", stackTraceLength);
	ret = sendwait(sizeof(int) + stackTraceLength);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (stack trace)");
};

NodeList<int> *getStackTrace(OSContext *context) {
	NodeList<int> *stackTrace = NULL;
	uint32_t stackPointer = context->gpr[1];
	uint32_t stackPointersCount = 0;

	while (isValidStackPointer(stackPointer)) {
		stackPointer = *(uint32_t *) stackPointer;
		if (!isValidStackPointer(stackPointer)) {
			break;
		}

		int data =(int) *(uint32_t *)(stackPointer + 4);
		stackTrace->insert(data);
		stackPointersCount++;
	}

	return stackTrace;
}

bool isValidStackPointer(uint32_t stackPointer) {
	return stackPointer >= 0x10000000 && stackPointer < 0x20000000;
}

/*
typedef struct _OsSpecifics
{
    unsigned int addr_OSDynLoad_Acquire;
    unsigned int addr_OSDynLoad_FindExport;
    unsigned int addr_OSTitle_main_entry;

    unsigned int addr_KernSyscallTbl1;
    unsigned int addr_KernSyscallTbl2;
    unsigned int addr_KernSyscallTbl3;
    unsigned int addr_KernSyscallTbl4;
    unsigned int addr_KernSyscallTbl5;
} OsSpecifics;

#define OS_SPECIFICS                ((OsSpecifics*)(0x00800000 + 0x1500))
*/

void CommandHandler::commmand_get_entry_point_address(){
	//uint32_t *entryPointAddress = (uint32_t *)*((uint32_t *)OS_SPECIFICS->addr_OSTitle_main_entry);
	uint32_t entryPointAddress = 0x1005E040;
	((uint32_t *)buffer)[0] = entryPointAddress;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Entry point address)");
};