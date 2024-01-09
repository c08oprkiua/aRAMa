//TCPGecko includes
#include "tcpgecko/title.h"
#include "tcpgecko/tcp_gecko.h"
#include "utils/logger.h"


#include <gctypes.h>

#include <coreinit/filesystem.h>

//aRAMa code
#include "arama.h"
#include "code_storage.h"
#include <wups.h>
#include <wups/storage.h>
#include <notifications/notifications.h>

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
WUPS_PLUGIN_AUTHOR("Rewrite & Aroma port: c08o.prkiua \n Original legacy version: wj444 + contributors");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_WUT_MALLOC(); //Idrk
WUPS_USE_STORAGE("aRAMa");

//General settings variables

INITIALIZE_PLUGIN(){
	InitaRAMaSettings();
	//Check if aRAMa is running offline or not
	if (arama_settings && 8){
		//TODO: Divorce Gecko functionality from networking
		startTCPGecko();
	}
}

DEINITIALIZE_PLUGIN(){

}

//Based on entry.c
ON_APPLICATION_START(){
	if ((arama_settings && 1) && isRunningAllowedTitleID()){
		log_print("OSGetTitleID checks passed...\n");
			if (arama_settings && 2){
			//Check titleid for codes
			//if the titleID has codes stored for it, load them
			}
	}
	//Todo: Figure out what all TCPGecko does on an application launch:

	//TCPGecko apparently would relaunch itself every time you open a title (maybe change that)

	//Apply game specific stored codes if that is enabled
}

ON_APPLICATION_REQUESTS_EXIT(){
	//Prolly some code that unpatches the game specific function patches TCPGecko employs
	//Idea: Clean up/remove ASM codes, cause leaving those executing might break stuff
}

WUPS_GET_CONFIG(){

}