#include "../command_handler.h"

#include "assertions.h"
//#include "../dynamic_libs/os_functions.h"
#include <coreinit/dynload.h>

//#include "../common/fs_defs.h"
#include <coreinit/filesystem.h>

//#include "kernel.h"
#include <kernel/kernel.h>

//audio video manager
#include <avm/drc.h>

void CommandHandler::command_pause_console(){
	writeConsoleState(PAUSED);
};

void CommandHandler::command_resume_console(){
	writeConsoleState(RUNNING);
};

void CommandHandler::command_is_console_paused(){
	bool paused = isConsolePaused();
	WHBLogPrintf("Paused: %d\n", paused);
	ret = sendByte((unsigned char)paused);
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
	unsigned int patchAddress = getConsoleStatePatchAddress();
	WHBLogPrintf("Patch address: %x\n", patchAddress);
	GeckoKernelCopyData((unsigned char *) patchAddress, (unsigned char *) &patchValue, 4);
}

bool isConsolePaused() {
	unsigned int patchAddress = getConsoleStatePatchAddress();
	WHBLogPrintf("Patch address: %x\n", patchAddress);
	int value = *(unsigned int *) patchAddress;

	return value == PAUSED;
}