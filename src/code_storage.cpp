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

//it has to return int or WUPS config complains, lol
int MakeWUPSGui(){

    //First we register the name of the plugin in WUPS base menu
    WUPSConfigHandle base;
    WUPSConfig_CreateHandled(&base, "aRAMa");

    //Then we register categories, such as the settings for aRAMa
    WUPSConfigCategoryHandle arama_category;
    WUPSConfig_AddCategoryByNameHandled(base, "aRAMa settings", &arama_category);

    WUPSConfigCategoryHandle game_codes;
    //Todo: Have it say the name of the app here instead of "this title"
    WUPSConfig_AddCategoryByNameHandled(base, "Codes for this title", &game_codes);

    WUPSConfigItemBoolean_AddToCategoryHandled(base, arama_category, "is_active", "aRAMa active",(arama_settings & ARAMA_SET_ACTIVE), &setting_changed);




    return 0;
}

void setting_changed(ConfigItemBoolean* item, bool new_value){
    //Determine which setting it is, edit accordingly, save
    //switch (item->configId){

    //}

}

//This will generate the WUPS Config screen per title for the game specific codes
void GetCodesByTitle(){
    uint64_t current_title_id = OSGetTitleID();
    uint8_t iterate = 1;
}

void StoreSentCode(){

}