#include <wups/storage.h>
#include <wups.h>
#include <whb/log.h>

#include "config.h"
#include "arama.h"

//certified C++ moment 
const char aRAMaConfig::active_id = '0';
const char aRAMaConfig::sd_codes_id = '1';
const char aRAMaConfig::notifs_id = '2';
const char aRAMaConfig::autosave_id = '4';
const char aRAMaConfig::caffiine_id = '5';
const char aRAMaConfig::saviine_id = '6';

int32_t aRAMaConfig::settings = 0;
uint32_t aRAMaConfig::ip_address = false;
uint32_t aRAMaConfig::local_code_amount = 0;

bool aRAMaConfig::active = false;
bool aRAMaConfig::sd_codes = false;
bool aRAMaConfig::notifications_enabled = false;
bool aRAMaConfig::no_online = false;
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

    active = settings & ACTIVE;
    sd_codes = settings & SD_CODES_ACTIVE;
    notifications_enabled = settings & NOTIFICATIONS_ON;
    no_online = settings & NO_ONLINE;
    auto_save = settings & AUTO_STORE_CODES;
    caffiine = settings & ENABLE_CAFFIINE;
    saviine = settings & ENABLE_SAVIINE;
}

void aRAMaConfig::SaveSettings(){
    if (WUPS_StoreInt(nullptr, "arama_settings", settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Saving aRAMa settings failed!");
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
            new_value ? settings |= ACTIVE : settings &= ~ACTIVE;
            active = new_value;
            break;
        case sd_codes_id:
            new_value ? settings |= SD_CODES_ACTIVE : settings &= ~SD_CODES_ACTIVE;
            sd_codes = new_value;
            break;
        case notifs_id:
            new_value ? settings |= NOTIFICATIONS_ON : settings &= ~NOTIFICATIONS_ON;
            notifications_enabled = new_value;
            break;
        case 3:

            no_online = new_value;
            break;
        case autosave_id:
            new_value ? settings |= AUTO_STORE_CODES : settings &= ~AUTO_STORE_CODES;
            auto_save = new_value;
            break;
        case caffiine_id:
            new_value ? settings |= ENABLE_CAFFIINE : settings &= ~ENABLE_CAFFIINE;
            caffiine = new_value;
            break;
        case saviine_id:
            new_value ? settings |= ENABLE_SAVIINE : settings &= ~ENABLE_SAVIINE;
            saviine = new_value;
            break;
    }
}