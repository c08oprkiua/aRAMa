#include "code_handler.h"
#include "arama.h"


void CodeHandler::loadSDCodes(){

}

void CodeHandler::executeAssembly(){
	int startAddress = getStartAddress();
	//log_printf("[Execute assembly] Start address: %i\n", startAddress);
	int startAddressInstruction = *(int*)startAddress;

	if (startAddressInstruction != 0) {
		void (*function)() = (void (*)()) startAddress;
		function();
		//log_print("[Execute assembly] Executed!\n");
	}
}

void CodeHandler::persistAssembly(uint32_t asm_size, uint8_t *source_buffer){
	assemblySize = asm_size;

	// Write the assembly to an executable code region

	//Todo: See if this works in Aroma (probably not) and fix it to work if not

	uint32_t startAddress = singletons.code_handler->getStartAddress();
	GeckoKernelCopyData((uint8_t *) startAddress, source_buffer, assemblySize);
}

int CodeHandler::run(){
	WHBLogPrint("Code handler installed...\n");
	void (*codeHandlerFunction)() = (void (*)()) CODE_HANDLER_INSTALL_ADDRESS;

	while (running) {
		usleep(9000);

	    // considerApplyingSDCheats();
		WHBLogPrint("Running code handler...\n");
		codeHandlerFunction();
		WHBLogPrint("Code handler done executing...\n");

		if (assemblySize > 0) {
			executeAssembly();
		}

		if (aRAMaConfig::sd_codes) {
			loadSDCodes(); //TODO: Only load an auto-load list
		}
	}
	return 0;
}

void CodeHandler::clear(){
	assemblySize = 0;
	WHBLogPrint("[Clear assembly] Assembly size 0\n");
}

uint32_t CodeHandler::getStartAddress() {
	return ENDING_ADDRESS - assemblySize;
}