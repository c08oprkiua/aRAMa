#include "arama.h"
#include "code_storage.h"

#include "wups/storage.h"

void InitaRAMaSettings(){
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS){
        //*mutter mutter* debug print
        return;
    }
    if (WUPS_GetInt(nullptr, "settings", &arama_settings) != WUPS_STORAGE_ERROR_SUCCESS){
        WUPS_StoreInt(nullptr, "settings", 5) //aRAMa is active, notifications on
    }
}