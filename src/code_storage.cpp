#include "arama.h"
#include "code_storage.h"

#include "wups/storage.h"

#include <coreinit/title.h>

//Code name convention: "code(int)", starting at int =1



void InitaRAMaSettings(){
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        //*mutter mutter* debug print
        return;
    }
    if (WUPS_GetInt(nullptr, "settings", &arama_settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WUPS_StoreInt(nullptr, "settings", 5); //aRAMa is active, notifications on
    }
}

void GetCodesByTitle(){
    uint64_t current_title_id = OSGetTitleID();
    uint8_t iterate = 1;
    
    
}

void StoreSentCode(){

}