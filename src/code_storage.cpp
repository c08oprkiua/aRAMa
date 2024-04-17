#include <wups/storage.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <coreinit/title.h>

#include "code_storage.h"
#include "arama.h"

//Code name convention: "code(int)", starting at int = 1

void InitaRAMaSettings(){
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Opening WUPS storage to retrieve settings failed!");
        return;
    }
    if (WUPS_GetInt(nullptr, "arama_settings", (int32_t) arama_settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Retrieving settings failed, writing default settings to storage...");
        WUPS_StoreInt(nullptr, "arama_settings", arama_settings);
    }
}

void SaveaRAMaSettings(){
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Opening WUPS storage to retrieve settings failed!");
    }
    else if (WUPS_StoreInt(nullptr, "arama_settings", (int32_t) arama_settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WHBLogPrint("Saving aRAMa settings failed!");
    }
}

//Big props to Inkay for serving as a model of how to do this stuff
int aRAMaConfig::LoadBaseConfigMenu(){

    //First we register the name of the plugin in WUPS base menu
    WUPSConfig_CreateHandled(&base, "aRAMa");

    //Then we register categories, such as the settings for aRAMa
    WUPSConfig_AddCategoryByNameHandled(base, "aRAMa settings", &arama_category);

    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "0", "aRAMa active",(arama_settings & ARAMA_SET_ACTIVE), &setting_changed, "Active", "Inactive");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "1", "Use SD codes", (arama_settings & ARAMA_SET_SD_CODES_ACTIVE), &setting_changed, "Yes", "No");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "2", "Notifications", (arama_settings & ARAMA_SET_NOTIFICATIONS_ON), &setting_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "3", "Operation mode", (arama_settings & ARAMA_SET_NO_ONLINE), &setting_changed, "Offline", "Online");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "4", "Save sent codes", (arama_settings & ARAMA_SET_AUTO_STORE_CODES), &setting_changed, "Save codes", "Don't save codes");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "5", "Caffiine", (arama_settings & ARAMA_ENABLE_CAFFIINE), &setting_changed, "Enabled", "Disabled");
    WUPSConfigItemBoolean_AddToCategoryHandledEx(base, arama_category, "6", "Saviine", (arama_settings & ARAMA_ENABLE_SAVIINE), &setting_changed, "Enabled", "Disabled");

    return 0;
}

void setting_changed(ConfigItemBoolean* item, bool new_value){
    //Determine which setting it is, edit accordingly, save
    char val_char = (char) item->configId;
    //Todo: look up char int as string values for this
    switch (val_char){
        case 0:
        
    }
    
    
}

//This will generate the WUPS Config screen per title for the game specific codes
int aRAMaConfig::LoadCodesForCurrentTitle(){
    uint64_t current_title_id = OSGetTitleID();
    uint8_t iterate = 1;

    //Todo: Have it say the name of the app here instead of "this title"
    WUPSConfig_AddCategoryByNameHandled(base, "Codes for this title", &codes_category);

    return 0;
}