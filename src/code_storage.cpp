#include "arama.h"
#include "code_storage.h"

#include <wups/storage.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <coreinit/title.h>

//Code name convention: "code(int)", starting at int = 1

void InitaRAMaSettings(){
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Opening WUPS storage to retrieve settings failed!");
        return;
    }
    if (WUPS_GetInt(nullptr, "arama_settings", (uint32_t *) arama_settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Retrieving settings failed, writing default settings to storage...");
        WUPS_StoreInt(nullptr, "arama_settings", arama_settings);
    }
}

void SaveaRAMaSettings(){
        if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Opening WUPS storage to retrieve settings failed!");
        return;
    }
    if (WUPS_StoreInt(nullptr, "arama_settings", arama_settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Saving aRAMa settings failed!");
    }
}

//Big props to Inkay for serving as a model of how to do this stuff
int aRAMaConfig::LoadBaseConfigMenu(){

    //First we register the name of the plugin in WUPS base menu
    WUPSConfig_CreateHandled(&base, "aRAMa");

    //Then we register categories, such as the settings for aRAMa
    WUPSConfig_AddCategoryByNameHandled(base, "aRAMa settings", &arama_category);

    //THIS IS PLACEHOLDER CODE
    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "is_active", "aRAMa active",(arama_settings & ARAMA_SET_ACTIVE), &setting_changed);
    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "sd_codes", "Use SD codes", (arama_settings & ARAMA_SET_SD_CODES_ACTIVE), &setting_changed);
    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "notis_on", "Enable notifications", (arama_settings & ARAMA_SET_NOTIFICATIONS_ON), &setting_changed);
    //Maybe use the custom fields of CategoryEx to make this a bit more clear, with "offline" and "online"
    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "be_offline", "Operate offline", (arama_settings & ARAMA_SET_NO_ONLINE), &setting_changed);
    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "autosave_codes", "Automatically save sent codes", (arama_settings & ARAMA_SET_AUTO_STORE_CODES), &setting_changed);
    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "caffiine_on", "Enable Caffiine", (arama_settings & ARAMA_ENABLE_CAFFIINE), &setting_changed);
    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "saviine_on", "Enable Saviine", (arama_settings & ARAMA_ENABLE_SAVIINE), &setting_changed);

    return 0;
}

void setting_changed(ConfigItemBoolean* item, bool new_value){
    //Determine which setting it is, edit accordingly, save
    uint32_t val_int; //=item->...something
}

//This will generate the WUPS Config screen per title for the game specific codes
int aRAMaConfig::LoadCodesForCurrentTitle(){
    uint64_t current_title_id = OSGetTitleID();
    uint8_t iterate = 1;

    //Todo: Have it say the name of the app here instead of "this title"
    WUPSConfig_AddCategoryByNameHandled(base, "Codes for this title", &codes_category);

    return 0;
}