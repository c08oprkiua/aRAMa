#pragma once

#include "kernel.h"

//libkernel
#include <kernel/kernel.h>

#define ENDING_ADDRESS 0x10000000
unsigned int assemblySize = 0;

unsigned int getStartAddress() {
	return ENDING_ADDRESS - assemblySize;
}

void persistAssembly(unsigned char buffer[], unsigned int size) {
	assemblySize = size;

	// Write the assembly to an executable code region
	unsigned int startAddress = getStartAddress();
	GeckoKernelCopyData((unsigned char *) startAddress, buffer, assemblySize);
}

void executeAssembly() {
	int startAddress = getStartAddress();
	log_printf("[Execute assembly] Start address: %i\n", startAddress);
	void (*function)() = (void (*)()) startAddress;
	function();
	log_print("[Execute assembly] Executed!\n");
}

void clearAssembly() {
	assemblySize = 0;
	log_print("[Clear assembly] Assembly size 0\n");
}
