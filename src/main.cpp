//TCPGecko includes
#include "tcpgecko/tcp_gecko.h"
#include "utils/logger.h"

//aRAMa code
#include "arama.h"
#include "code_storage.h"

#include <coreinit/filesystem.h>
#include <wups.h>
#include <wups/storage.h>
//#include <notifications/notifications.h>

//Metadata
WUPS_PLUGIN_NAME("aRAMa");
WUPS_PLUGIN_DESCRIPTION("RAM magic for Aroma");
/*A memory editor that does magical things to your games. In order to develop and apply real-time cheats use JGecko U.

        Special thanks to:
        Chadderz, Marionumber1 - Original TCP Gecko Installer
        dimok - Homebrew Launcher
        kinnay - Diibugger
        pwsincd - Icon and XML
        CosmoCortney - Original Cheat code handler
        Mewtality - New Cheat code handler
- from meta.xml
*/
WUPS_PLUGIN_VERSION("v0.1");
WUPS_PLUGIN_AUTHOR("aRAMa: c08o.prkiua \n TCPGecko: BullyWiiPlaza + contributors");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_WUT_MALLOC(); //Idk
WUPS_USE_STORAGE("aRAMa");

INITIALIZE_PLUGIN(){
	/*
	InitOSFunctionPointers();
	InitSocketFunctionPointers();
	InitGX2FunctionPointers();

	log_init(COMPUTER_IP_ADDRESS);
	startTCPGecko();
		setup_os_exceptions();
		socket_lib_init();
		initializeUDPLog();
	*/
	InitaRAMaSettings();
	aRAMaReInit(); //"Re"Init, but init and reinit are identical, so
}

DEINITIALIZE_PLUGIN(){
	aRAMaDeInit();
}

void aRAMaReInit(){
	//If aRAMa shoudn't be active, immedeately end function
	if (!(arama_settings & ARAMA_SET_ACTIVE)){
		return;
	}
	
	//SD codes get priority, regardless of being online
	if (arama_settings & ARAMA_SET_SD_CODES_ACTIVE){
		//Init Gecko so it can load SD codes

		log_print("aRAMa is active, checking for local codes for this title...\n");

		//Check SD codes

		//deinit if offline and no codes are found
	}
	if (arama_settings & ARAMA_SET_NO_ONLINE){
		isOnline = false;
		//If aRAMa is active and offline, but SD codes are disabled, no reason
		//to keep it loaded in cause it'll sit there doing nothing
		if (!(arama_settings & ARAMA_SET_SD_CODES_ACTIVE)){
			aRAMaDeInit();
		}
		//Start aRAMa offline, with only SD codes
		return;
	}
	//If we're not offline, we must be online :bigbrain:
	else{
		
		//This setting has not changed, and it returns the opposite of the previous check,
		//So it can be used to check for Gecko being initialized
		if (!(arama_settings && ARAMA_SET_SD_CODES_ACTIVE)){
			//Gecko was not initialized earlier, so init now
			//Init Gecko
		}
		if (TCPSetUp == false){
			//Init TCP function
		}
	}
}

void aRAMaDeInit(){
	//Something something free GeckoProcessor
}

ON_APPLICATION_START(){
	aRAMaReInit();
	//Todo: Figure out what all TCPGecko does on an application launch:

	//TCPGecko apparently would relaunch itself every time you open a title (maybe change that)

}

ON_APPLICATION_REQUESTS_EXIT(){
	//Prolly clean up/remove ASM codes, cause leaving those executing might break stuff
}

WUPS_GET_CONFIG(){

}

WUPS_CONFIG_CLOSED(){
	//Save settings, and then
	aRAMaReInit();
}