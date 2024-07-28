#include "../command_handler.h"

#include <coreinit/dynload.h>

#include <coreinit/filesystem.h>
#include <coreinit/internal.h>

#include <kernel/kernel.h>

//audio video manager
#include <avm/drc.h>

#define ASSERT_FUNCTION_SUCCEEDED(returnValue, functionName) \
    if (returnValue < 0) { \
        char buffer[100] = {0}; \
        __os_snprintf(buffer, 100, "%s failed with return value: %i", functionName, returnValue); \
        OSFatal(buffer); \
    } \

void CommandHandler::command_pause_console(){
	writeConsoleState(PAUSED);
};

void CommandHandler::command_resume_console(){
	writeConsoleState(RUNNING);
};

void CommandHandler::command_is_console_paused(){
	bool paused = isConsolePaused();
	WHBLogPrintf("Paused: %d\n", paused);
	ret = sendByte((uint8_t)paused);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendByte (sending paused console status)")
};

unsigned long getConsoleStatePatchAddress() {
	AVMDrcScanMode mode;
	AVMGetDRCScanMode(&mode);

	return (unsigned long) ((char *) mode + 0x44);
}

typedef enum {
	PAUSED = 0x38000001,
	RUNNING = 0x38000000
} ConsoleState;

//Before you ask: no, I don't know what voodoo is happening here
void writeConsoleState(ConsoleState state) {
	// Get the value to write
	int patchValue = state;
	WHBLogPrintf("Patch value: %x\n", patchValue);

	// Write the value
	uint32_t patchAddress = getConsoleStatePatchAddress();
	WHBLogPrintf("Patch address: %x\n", patchAddress);
	GeckoKernelCopyData((uint8_t *) patchAddress, (uint8_t *) &patchValue, 4);
}

bool isConsolePaused() {
	uint32_t patchAddress = getConsoleStatePatchAddress();
	WHBLogPrintf("Patch address: %x\n", patchAddress);
	int value = *(uint32_t *) patchAddress;

	return value == PAUSED;
}