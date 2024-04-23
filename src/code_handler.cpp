#include "code_handler.h"
#include "arama.h"


void CodeHandler::loadSDCodes(){

}

void CodeHandler::executeAssembly(){
    
}

void CodeHandler::run(){
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
}