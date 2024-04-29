#include <nsysnet/socket.h>
#include <gx2/event.h>

#include "arama.h"



#define CODE_HANDLER_INSTALL_ADDRESS 0x010F4000

void aRAMaReInit(){
	//If aRAMa shoudn't be active, immedeately end function
	if (!(aRAMaConfig::active)){

		aRAMaDeInit();
		return;
	}
	
	//Init gecko here


	//SD codes get priority, regardless of being online
	if (aRAMaConfig::sd_codes){
		//Init Gecko so it can load SD codes

		WHBLogPrint("aRAMa is active, checking for local codes for this title...\n");

		//Check SD codes

		//deinit if offline and no codes are found
	}

	if (!aRAMaConfig::tcpgecko){
		
		if (!(aRAMaConfig::sd_codes)){
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
		if (!(aRAMaConfig::sd_codes)){
			//Run SD codes
		}

	}
	
	// Activate TCP Gecko if it's enabled and not already activated
	if (aRAMaConfig::tcpgecko && gecko == nullptr){

		gecko = new GeckoProcessor; //May need a memalign for performance?

		gecko->running = true;

		WHBLogPrint("Starting TCPGecko thread.\n");
		if (OSCreateThread(
		gecko->thread, 
		&GeckoProcessor::runGeckoServer, 
		1, 
		(char *) gecko,
		(gecko->stack + sizeof(gecko->stack)), 
		sizeof(gecko->stack), 
		0, 
		0xC
		) == true) {
		OSResumeThread(gecko->thread);
		}
		WHBLogPrint("TCP Gecko thread started...\n");
	}
}

void aRAMaDeInit(){
	if (!aRAMaConfig::tcpgecko && gecko != nullptr){
		gecko->running = false;
		int thread_result;
		OSJoinThread(gecko->thread, &thread_result);
		delete gecko;
		gecko = nullptr;
	}
	if (c_h != nullptr && !aRAMaConfig::code_handler){
		delete c_h;
		c_h = nullptr;
	}
	if (caffiine != nullptr && !aRAMaConfig::caffiine){
		delete caffiine;
		caffiine = nullptr;
	}
	if (saviine != nullptr && !aRAMaConfig::saviine){
		delete saviine;
		saviine = nullptr;
	}
}

