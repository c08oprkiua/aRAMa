#include <whb/log.h>

#include <coreinit/filesystem.h>
#include <wups.h>
//#include <wups/storage.h>
#include <whb/log_udp.h>

#include "arama.h"

//Metadata
WUPS_PLUGIN_NAME("aRAMa");
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
WUPS_PLUGIN_VERSION("v0.1");
WUPS_PLUGIN_AUTHOR("aRAMa: c08o.prkiua; TCPGecko: BullyWiiPlaza + contributors");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
//WUPS_USE_WUT_MALLOC(); //Idk
WUPS_USE_STORAGE("aRAMa");

INITIALIZE_PLUGIN(){
	WHBLogUdpInit();
	
	config = new aRAMaConfig;

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
}

DEINITIALIZE_PLUGIN(){
	aRAMaDeInit();
	delete config;
	WHBLogUdpDeinit();
}

ON_APPLICATION_START(){
	config->LoadSettings();
	aRAMaReInit();
}

ON_APPLICATION_REQUESTS_EXIT(){
	//Prolly clean up/remove ASM codes, cause leaving those executing might break stuff
}

WUPS_GET_CONFIG(){
	config->LoadBaseConfigMenu();
}

WUPS_CONFIG_CLOSED(){
	config->SaveSettings();
	aRAMaReInit();
}