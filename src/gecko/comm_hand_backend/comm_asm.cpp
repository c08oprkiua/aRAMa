#include "../command_handler.h"
#include <kernel/kernel.h>
#include <coreinit/debug.h>
//#include "../tcpgecko/kernel.h"

#define ENDING_ADDRESS 0x10000000
uint32_t assemblySize = 0; //Todo: this is in code_handler

uint8_t *kernelCopyBufferOld[DATA_BUFFER_SIZE];

//TODO: Make these interface with the code_handler

void CommandHandler::command_execute_assembly(){
	receiveString((uint8_t *)buffer, DATA_BUFFER_SIZE);
	executeAssembly(buffer, DATA_BUFFER_SIZE);
};

void CommandHandler::command_persist_assembly(){
	uint32_t length = receiveString((uint8_t *)buffer, DATA_BUFFER_SIZE);
	assemblySize = length;

	// Write the assembly to an executable code region

	//Todo: See if this works in Aroma (probably not) and fix it to work if not
	uint32_t startAddress = getStartAddress();
	GeckoKernelCopyData((uint8_t *) startAddress, buffer, assemblySize);
	
};

void CommandHandler::command_clear_assembly(){
	assemblySize = 0;
	WHBLogPrint("[Clear assembly] Assembly size 0\n");
};

uint32_t getStartAddress() {
	return ENDING_ADDRESS - assemblySize;
}

//Todo: Move this somewhere more universally accessible
void GeckoKernelCopyData(uint8_t * dest, uint8_t * source, uint32_t length){
	if (length > DATA_BUFFER_SIZE){
		OSFatal("Kernel copy buffer size exceeded");
	}

	memcpy(kernelCopyBufferOld, source, length);
	KernelCopyData((uint32_t) OSEffectiveToPhysical(dest), (uint32_t) &kernelCopyBufferOld, length);

	DCFlush(dest, length);
}