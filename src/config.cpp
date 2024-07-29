#include <coreinit/title.h>
#include <wups/storage.h>
#include <wups.h>
#include "config.h"
#include "arama.h"

//certified C++ moment
const char aRAMaConfig::active_id = '0';
const char aRAMaConfig::sd_codes_id = '1';
const char aRAMaConfig::notifs_id = '2';
const char aRAMaConfig::code_hand_id = '3';
const char aRAMaConfig::autosave_id = '4';
const char aRAMaConfig::caffiine_id = '5';
const char aRAMaConfig::saviine_id = '6';
const char aRAMaConfig::tcpgecko_id = '7';

uint32_t aRAMaConfig::settings = 0;
uint32_t aRAMaConfig::ip_address = false;
uint32_t aRAMaConfig::local_code_amount = 0;

aRAMaConfig::aRAMaConfig(){
	LoadSettings();
	SaveSettings();
}

aRAMaConfig::~aRAMaConfig(){
    settings = 0;
    plugin_cleanup();
}

bool aRAMaConfig::get_plugin_active(){
    return settings & ACTIVE;
}

void aRAMaConfig::set_plugin_active(bool new_plugin_active){
    new_plugin_active ? settings |= ACTIVE : settings &= ~ACTIVE;
}

bool aRAMaConfig::get_sd_codes(){
    return settings & SD_CODES_ACTIVE;
}

void aRAMaConfig::set_sd_codes(bool new_sd_codes){
    new_sd_codes ? settings |= SD_CODES_ACTIVE : settings &= ~SD_CODES_ACTIVE;;
}

bool aRAMaConfig::get_notifs_on(){
    return settings & NOTIFICATIONS_ON;
}

void aRAMaConfig::set_notifs_on(bool new_notifs_on){
    new_notifs_on ? settings |= NOTIFICATIONS_ON : settings &= ~NOTIFICATIONS_ON;
}

bool aRAMaConfig::get_code_autosave(){
    return settings & AUTO_STORE_CODES;
}

void aRAMaConfig::set_code_autosave(bool new_code_autosave){
    new_code_autosave ? settings |= AUTO_STORE_CODES : settings &= ~AUTO_STORE_CODES;
}

bool aRAMaConfig::get_tcpgecko(){
    return settings & ENABLE_TCPGECKO;
}

void aRAMaConfig::set_tcpgecko(bool new_tcpgecko){
    new_tcpgecko ? settings |= ENABLE_TCPGECKO : settings &= ~ENABLE_TCPGECKO;
}

bool aRAMaConfig::get_code_handler(){
    return settings & CODE_HANDLER;
}

void aRAMaConfig::set_code_handler(bool new_code_handler){
    new_code_handler ? settings |= CODE_HANDLER : settings &= ~CODE_HANDLER;
}

bool aRAMaConfig::get_caffiine(){
    return settings & ENABLE_CAFFIINE;
}

void aRAMaConfig::set_caffiine(bool new_caffiine){
    new_caffiine ? settings |= ENABLE_CAFFIINE : settings &= ~ENABLE_CAFFIINE;
}

bool aRAMaConfig::get_saviine(){
    return settings & ENABLE_SAVIINE;
}

void aRAMaConfig::set_saviine(bool new_saviine){
    new_saviine ? settings |= ENABLE_SAVIINE : settings &= ~ENABLE_SAVIINE;;
}


void aRAMaConfig::LoadSettings(){
    /*if (WUPSStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        singletons.log->print("aRAMa: Opening WUPS storage failed!");
        return;
    }*/
    if (WUPSStorageAPI_GetU32(nullptr, "arama_settings", &settings) != WUPS_STORAGE_ERROR_SUCCESS){
        singletons.log->print("aRAMa: Retrieving settings failed");
    }
}

void aRAMaConfig::SaveSettings(){
    if (WUPSStorageAPI_StoreU32(nullptr, "arama_settings", settings) != WUPS_STORAGE_ERROR_SUCCESS){
        singletons.log->print("aRAMa: Error occured in storing settings!");
    }
    if (WUPSStorageAPI_SaveStorage(false) != WUPS_STORAGE_ERROR_SUCCESS) {
        singletons.log->print("aRAMa: Error occured in saving settings!");
    }
}

void aRAMaConfig::settings_changed(ConfigItemBoolean* item, bool new_value){
    char val_char = *item->identifier;
    switch (val_char){
        case active_id:
            singletons.config->set_plugin_active(new_value);
            break;
        case sd_codes_id:
            singletons.config->set_sd_codes(new_value);
            break;
        case notifs_id:
            singletons.config->set_notifs_on(new_value);
            break;
        case code_hand_id:
            singletons.config->set_code_handler(new_value);
            break;
        case autosave_id:
            singletons.config->set_code_autosave(new_value);
            break;
        case caffiine_id:
            singletons.config->set_caffiine(new_value);
            break;
        case saviine_id:
            singletons.config->set_saviine(new_value);
            break;
    }
}

void aRAMaConfig::plugin_setup(){
	//If aRAMa shoudn't be active, immedeately end function
	if (!get_plugin_active()){
		plugin_cleanup();
		return;
	}

	//SD codes get priority, regardless of being online
	if (get_sd_codes()){
		//Init Gecko so it can load SD codes

		singletons.log->print_w_notif("aRAMa is active, checking for local codes for this title...\n", 8);

		//Check SD codes

		//deinit if offline and no codes are found
	}

	if (!get_tcpgecko()){
		if (!get_sd_codes()){
			//If aRAMa is active and offline, but SD codes are disabled, no reason
			//to keep it loaded in cause it'll sit there doing nothing
			plugin_cleanup();
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
		if (!get_sd_codes()){
			//Run SD codes
		}
	}
	
	// Activate TCP Gecko if it's enabled and not already activated
	if (get_tcpgecko() && singletons.gecko == nullptr){
		singletons.gecko = new GeckoProcessor;
		singletons.gecko->start_server();
	}
}

void aRAMaConfig::plugin_cleanup(){
	if (!get_tcpgecko() && singletons.gecko != nullptr){
		singletons.gecko->stop_server();
		delete singletons.gecko;
		singletons.gecko = nullptr;
	}
	if (singletons.code_handler != nullptr && !get_code_handler()){
        singletons.code_handler->stop_server();
		delete singletons.code_handler;
		singletons.code_handler = nullptr;
	}
	if (singletons.caffiine != nullptr && !get_caffiine()){
        singletons.caffiine->stop_server();
		delete singletons.caffiine;
		singletons.caffiine = nullptr;
	}
	if (singletons.saviine != nullptr && !get_saviine()){
        singletons.saviine->stop_server();
		delete singletons.saviine;
		singletons.saviine = nullptr;
	}
}

WUPSConfigAPICallbackStatus aRAMaConfig::open_config(WUPSConfigCategoryHandle root){
    LoadSettings();
    WUPSConfigCategoryHandle base_category;

    WUPSConfigAPICreateCategoryOptionsV1 base_opts;

    base_opts.name = ARAMA_PLUGIN_NAME;

    WUPSConfigAPI_Category_Create(base_opts, &base_category);

    WUPSConfigCategoryHandle arama_category;
    WUPSConfigAPICreateCategoryOptionsV1 arama_opts;
    arama_opts.name = "aRAMa settings";
    
    WUPSConfigAPI_Category_Create(arama_opts, &arama_category);
    

    //Then we register categories, such as the settings for aRAMa
    WUPSConfigAPI_Category_AddCategory(base_category, arama_category);
    
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &active_id, "aRAMa active", true, get_plugin_active(), &settings_changed, "Active", "Inactive");
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &sd_codes_id, "Use SD codes", false, get_sd_codes(), &settings_changed, "Yes", "No");
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &notifs_id, "Notifications", false, get_notifs_on(), &settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &autosave_id, "When recieving codes:", false, get_code_autosave(), &settings_changed, "Save codes", "Don't save codes");
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &tcpgecko_id, "TCPGecko", true, get_tcpgecko(), &settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &code_hand_id, "Code handler", true, get_code_handler(), &settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &caffiine_id, "Caffiine", false, get_caffiine(), &settings_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryEx(arama_category, &saviine_id, "Saviine", false, get_saviine(), &settings_changed, "Enabled", "Disabled");

    //If there are SD codes and it's enabled, lode that category too
    if (aRAMaConfig::get_sd_codes() && local_code_amount > 0){
        uint64_t current_title_id = OSGetTitleID();
        uint8_t iterate = 1;

        WUPSConfigCategoryHandle codes_category;

        //Todo: Have it say the name of the app here instead of "this title"
        //WUPSConfig_AddCategoryByNameHandled(base, "SD Codes for this title", &codes_category);
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void aRAMaConfig::close_config(){
	SaveSettings();
    plugin_cleanup();
	plugin_setup();
}