#pragma once

#include "kernel.h"

//libkernel
#include <kernel/kernel.h>

#define ENDING_ADDRESS 0x10000000
unsigned int assemblySize = 0;

unsigned int getStartAddress() {
	return ENDING_ADDRESS - assemblySize;
}

void executeAssembly() {
	int startAddress = getStartAddress();
	WHBLogPrintf("[Execute assembly] Start address: %i\n", startAddress);
	void (*function)() = (void (*)()) startAddress;
	function();
	WHBLogPrint("[Execute assembly] Executed!\n");
}