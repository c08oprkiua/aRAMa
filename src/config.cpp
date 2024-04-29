#include <wups/storage.h>
#include <wups.h>
#include <whb/log.h>

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


int32_t aRAMaConfig::settings = 0;
int32_t aRAMaConfig::settings_live_cache = 0;
uint32_t aRAMaConfig::ip_address = false;
uint32_t aRAMaConfig::local_code_amount = 0;

bool aRAMaConfig::active = false;
bool aRAMaConfig::sd_codes = false;
bool aRAMaConfig::notifications_enabled = false;
bool aRAMaConfig::auto_save = false;

bool aRAMaConfig::tcpgecko = false;
bool aRAMaConfig::code_handler = false;
bool aRAMaConfig::caffiine = false;
bool aRAMaConfig::saviine = false;


void aRAMaConfig::LoadSettings(){
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("aRAMa: Opening WUPS storage failed!");
        return;
    }
    if (WUPS_GetInt(nullptr, "arama_settings", &settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("aRAMa: Retrieving settings failed");
    }
    settings_live_cache = settings;
    
    active = settings & ACTIVE;
    sd_codes = settings & SD_CODES_ACTIVE;
    notifications_enabled = settings & NOTIFICATIONS_ON;
    code_handler = settings & CODE_HANDLER;
    auto_save = settings & AUTO_STORE_CODES;
    caffiine = settings & ENABLE_CAFFIINE;
    saviine = settings & ENABLE_SAVIINE;
}

void aRAMaConfig::SaveSettings(){
    if (settings_live_cache != settings){
        if (WUPS_StoreInt(nullptr, "arama_settings", settings) != WUPS_STORAGE_ERROR_SUCCESS){
            WHBLogPrint("Saving aRAMa settings failed!");
        }
    }
    if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        WHBLogPrint("aRAMa failed to close storage!");
    }
}


//Hope you know your bitwise operators :trollface:
void aRAMaConfig::settings_changed(ConfigItemBoolean* item, bool new_value){
    char val_char = *item->configId;
    switch (val_char){
        case active_id:
            new_value ? settings_live_cache |= ACTIVE : settings_live_cache &= ~ACTIVE;
            active = new_value;
            break;
        case sd_codes_id:
            new_value ? settings_live_cache |= SD_CODES_ACTIVE : settings_live_cache &= ~SD_CODES_ACTIVE;
            sd_codes = new_value;
            break;
        case notifs_id:
            new_value ? settings_live_cache |= NOTIFICATIONS_ON : settings_live_cache &= ~NOTIFICATIONS_ON;
            notifications_enabled = new_value;
            break;
        case code_hand_id:
            new_value ? settings_live_cache |= CODE_HANDLER : settings_live_cache &= ~CODE_HANDLER;
            code_handler = new_value;
            break;
        case autosave_id:
            new_value ? settings_live_cache |= AUTO_STORE_CODES : settings_live_cache &= ~AUTO_STORE_CODES;
            auto_save = new_value;
            break;
        case caffiine_id:
            new_value ? settings_live_cache |= ENABLE_CAFFIINE : settings_live_cache &= ~ENABLE_CAFFIINE;
            caffiine = new_value;
            break;
        case saviine_id:
            new_value ? settings_live_cache |= ENABLE_SAVIINE : settings_live_cache &= ~ENABLE_SAVIINE;
            saviine = new_value;
            break;
    }
}