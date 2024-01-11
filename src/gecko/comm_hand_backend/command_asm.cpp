#include "../command_handler.h"
#include "./tcpgecko/kernel.h"

void CommandHandler::command_execute_assembly(){
	receiveString((unsigned char *)buffer, DATA_BUFFER_SIZE);
	executeAssembly(buffer, DATA_BUFFER_SIZE);
};

void CommandHandler::command_persist_assembly(){
	unsigned int length = receiveString((unsigned char *)buffer, DATA_BUFFER_SIZE);
	assemblySize = length;

	// Write the assembly to an executable code region
	unsigned int startAddress = getStartAddress();
	GeckoKernelCopyData((unsigned char *) startAddress, buffer, assemblySize);
};

//clearAssembly
void CommandHandler::command_clear_assembly(){
	assemblySize = 0;
	log_print("[Clear assembly] Assembly size 0\n");
};

#define ENDING_ADDRESS 0x10000000
unsigned int assemblySize = 0;

unsigned int getStartAddress() {
	return ENDING_ADDRESS - assemblySize;
}