#include <nsysnet/socket.h>
#include <gx2/event.h>

#include "arama.h"

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
		singletons.code_handler->start_server();
		//Start aRAMa offline, with only SD codes
		return;
	}
	//If we're not offline, we must be online :bigbrain:
	else {
		singletons.gecko->start_server();
		//This setting has not changed, and it returns the opposite of the previous check,
		//So it can be used to check for Gecko being initialized
		if (!(aRAMaConfig::sd_codes)){
			//Run SD codes
		}

	}
	
	// Activate TCP Gecko if it's enabled and not already activated
	if (aRAMaConfig::tcpgecko && singletons.gecko == nullptr){

		singletons.gecko = new GeckoProcessor;

		singletons.gecko->start_server();
	}
}

void aRAMaDeInit(){
	if (!aRAMaConfig::tcpgecko && singletons.gecko != nullptr){
		singletons.gecko->stop_server();
		delete singletons.gecko;
		singletons.gecko = nullptr;
	}
	if (singletons.code_handler != nullptr && !aRAMaConfig::code_handler){
		delete singletons.code_handler;
		singletons.code_handler = nullptr;
	}
	if (singletons.caffiine != nullptr && !aRAMaConfig::caffiine){
		delete singletons.caffiine;
		singletons.caffiine = nullptr;
	}
	if (singletons.saviine != nullptr && !aRAMaConfig::saviine){
		delete singletons.saviine;
		singletons.saviine = nullptr;
	}
}

