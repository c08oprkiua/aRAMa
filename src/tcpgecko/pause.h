#pragma once

#include "../utils/logger.h"
#include "assertions.h"
//#include "../dynamic_libs/os_functions.h"
#include <coreinit/dynload.h>

//#include "../common/fs_defs.h"
#include <coreinit/filesystem.h>

//#include "kernel.h"
#include <kernel/kernel.h>

//audio video manager
#include <avm/drc.h>

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
	log_printf("Patch value: %x\n", patchValue);

	// Write the value
	unsigned int patchAddress = getConsoleStatePatchAddress();
	log_printf("Patch address: %x\n", patchAddress);
	GeckoKernelCopyData((unsigned char *) patchAddress, (unsigned char *) &patchValue, 4);
}

bool isConsolePaused() {
	unsigned int patchAddress = getConsoleStatePatchAddress();
	log_printf("Patch address: %x\n", patchAddress);
	int value = *(unsigned int *) patchAddress;

	return value == PAUSED;
}