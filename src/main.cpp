#include <whb/log.h>

#include <coreinit/filesystem.h>
#include <coreinit/title.h>
#include <wups.h>
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
WUPS_PLUGIN_VERSION("active-dev, v0.1");
WUPS_PLUGIN_AUTHOR("c08oprkiua, TCPGecko contributors");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
//WUPS_USE_WUT_MALLOC(); //Idk
WUPS_USE_STORAGE("aRAMa");

INITIALIZE_PLUGIN(){
	WHBLogUdpInit();
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
	aRAMaConfig::LoadSettings();
	aRAMaConfig::SaveSettings();
}

DEINITIALIZE_PLUGIN(){
	aRAMaDeInit();
	WHBLogUdpDeinit();
}

ON_APPLICATION_START(){
	if (aRAMaConfig::sd_codes){
		//TODO: count SD codes, update aRAMaConfig::local_codes_amount with amount
	}
	aRAMaReInit();
}

ON_APPLICATION_REQUESTS_EXIT(){
    aRAMaDeInit();
	//Prolly clean up/remove ASM codes, cause leaving those executing might break stuff
}

WUPS_GET_CONFIG(){
    aRAMaConfig::LoadSettings();
    WUPSConfigHandle base;

    WUPSConfig_CreateHandled(&base, "aRAMa");

    WUPSConfigCategoryHandle arama_category;
    //Then we register categories, such as the settings for aRAMa
    WUPSConfig_AddCategoryByNameHandled(base, "aRAMa settings", &arama_category);

    //WUPS may not like it when I use raw values for the IDs...
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, &aRAMaConfig::active_id, "aRAMa active", aRAMaConfig::active, &aRAMaConfig::settings_changed, "Active", "Inactive");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, &aRAMaConfig::sd_codes_id, "Use SD codes", aRAMaConfig::sd_codes, &aRAMaConfig::settings_changed, "Yes", "No");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, &aRAMaConfig::notifs_id, "Notifications", aRAMaConfig::notifications_enabled, &aRAMaConfig::settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, &aRAMaConfig::autosave_id, "Save sent codes", aRAMaConfig::auto_save, &aRAMaConfig::settings_changed, "Save codes", "Don't save codes");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "Z", "TCP Gecko", aRAMaConfig::active, &aRAMaConfig::settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, &aRAMaConfig::code_hand_id, "Code handler", aRAMaConfig::code_handler, &aRAMaConfig::settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, &aRAMaConfig::caffiine_id, "Caffiine", aRAMaConfig::caffiine, &aRAMaConfig::settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, &aRAMaConfig::saviine_id, "Saviine", aRAMaConfig::caffiine, &aRAMaConfig::settings_changed, "Enabled", "Disabled");

    //If there are SD codes and it's enabled, lode that category too
    if (aRAMaConfig::sd_codes && aRAMaConfig::local_code_amount > 0){
        uint64_t current_title_id = OSGetTitleID();
        uint8_t iterate = 1;

        WUPSConfigCategoryHandle codes_category;

        //Todo: Have it say the name of the app here instead of "this title"
        WUPSConfig_AddCategoryByNameHandled(base, "SD Codes for this title", &codes_category);
    }
    return base;
}

WUPS_CONFIG_CLOSED(){
	aRAMaConfig::SaveSettings();
    aRAMaDeInit();
	aRAMaReInit();
}