#include <nsysnet/socket.h>
#include <gx2/event.h>

#include "arama.h"

#define CODE_HANDLER_INSTALL_ADDRESS 0x010F4000

void aRAMaReInit(){
	//If aRAMa shoudn't be active, immedeately end function
	if (config == nullptr || !(config->active)){

		aRAMaDeInit();
		return;
	}
	
	//Init gecko here


	//SD codes get priority, regardless of being online
	if (config->sd_codes){
		//Init Gecko so it can load SD codes

		WHBLogPrint("aRAMa is active, checking for local codes for this title...\n");

		//Check SD codes

		//deinit if offline and no codes are found
	}

	if (config->no_online){
		
		if (!(config->sd_codes)){
			//If aRAMa is active and offline, but SD codes are disabled, no reason
			//to keep it loaded in cause it'll sit there doing nothing
			aRAMaDeInit();
			return;
		}
		//Start aRAMa offline, with only SD codes
		return;
	}
	//If we're not offline, we must be online :bigbrain:
	else {
		//This setting has not changed, and it returns the opposite of the previous check,
		//So it can be used to check for Gecko being initialized
		if (!(config->sd_codes)){
			//Run SD codes
		}

	}
}

void aRAMaDeInit(){
	if (gecko != nullptr && !config->tcpgecko){
		delete gecko;
		gecko = nullptr;
	}
	if (c_h != nullptr && !config->code_handler){
		delete c_h;
		c_h = nullptr;
	}
	if (caffiine != nullptr && !config->caffiine){
		delete caffiine;
		caffiine = nullptr;
	}
}

static int CreateGeckoThread(){
	if (gecko != nullptr){
		// Run the TCP Gecko Installer server

		/*
		bss = (struct pygecko_bss_t *) memalign(0x40, sizeof(struct pygecko_bss_t));
		if (bss == 0)
			return (s32) 0;
		memset(bss, 0, sizeof(struct pygecko_bss_t));
    	*/

		WHBLogPrint("Starting TCPGecko thread.\n");
		if (OSCreateThread(
		gecko->thread, 
		(OSThreadEntryPointFn) runGeckoServer, 
		(uint32_t) 0, 
		(char *) 0,
		(void *)(gecko->stack + sizeof(gecko->stack)), 
		sizeof(gecko->stack), 
		0, 
		0xC
		) == true) {
		OSResumeThread(gecko->thread);
		}
		WHBLogPrint("TCP Gecko thread started...\n");
	}

	//The code after here should be moved to the CodeHandler class in the future

	// Execute the code handler if it is installed
	if (c_h != nullptr) {
		WHBLogPrint("Code handler installed...\n");
		void (*codeHandlerFunction)() = (void (*)()) CODE_HANDLER_INSTALL_ADDRESS;

		while (true) {
			usleep(9000);

			// considerApplyingSDCheats();
			// WHBLogPrint("Running code handler...\n");
			codeHandlerFunction();
			// WHBLogPrint("Code handler done executing...\n");

			/* Assembly codes are not yet supported by aRAMa
			if (assemblySize > 0) {
				executeAssembly();
			}
			*/

			if (config->sd_codes) {
				//considerApplyingSDCheats();
			}
		}
	} else {
		WHBLogPrint("Code handler not installed...\n");
	}

	return 0;
}
