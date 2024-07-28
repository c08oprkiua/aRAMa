#include <whb/log.h>
#include <whb/log_udp.h>

#include <coreinit/filesystem.h>
#include <coreinit/title.h>
#include <wups.h>

#include "arama.h"

//Metadata
WUPS_PLUGIN_NAME(ARAMA_PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION("RAM multi-tool for Aroma");
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
WUPS_PLUGIN_VERSION("active-dev, v0.2");
WUPS_PLUGIN_AUTHOR("c08oprkiua, TCPGecko contributors");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
//WUPS_USE_WUT_MALLOC(); //Idk
WUPS_USE_STORAGE(ARAMA_PLUGIN_NAME);

WUPSConfigAPICallbackStatus arama_get_config(WUPSConfigCategoryHandle root){
    return singletons.config->open_config(root);
}

void arama_close_config() {
	singletons.config->close_config();
}

INITIALIZE_PLUGIN(){
	WHBLogUdpInit();

    WUPSConfigAPIOptionsV1 optionsV1;
    optionsV1.name = ARAMA_PLUGIN_NAME;

    WUPSConfigAPI_Init(optionsV1, arama_get_config, arama_close_config);
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
    singletons.config = new aRAMaConfig;
}

DEINITIALIZE_PLUGIN(){
	singletons.config->plugin_cleanup();
	WHBLogUdpDeinit();
}

ON_APPLICATION_START(){
	if (singletons.config->get_sd_codes()){
		//TODO: count SD codes, update aRAMaConfig::local_codes_amount with amount
	}
    singletons.config->plugin_setup();
}

ON_APPLICATION_REQUESTS_EXIT(){
    singletons.config->plugin_cleanup();
	//Prolly clean up/remove ASM codes, cause leaving those executing might break stuff
}