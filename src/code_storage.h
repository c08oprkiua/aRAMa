#include <wups/storage.h>
#include <wups.h>

#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config.h>

//Create/fetch all the settings in WUPS storage
void InitaRAMaSettings();
//Save the settings to WUPS storage
void SaveaRAMaSettings();


class aRAMaConfig {
public:

    WUPSConfigHandle base;
    WUPSConfigCategoryHandle arama_category;
    WUPSConfigCategoryHandle codes_category;


    //these have to return int or WUPS config complains, lol

    int LoadBaseConfigMenu();
    //Get codes based on the TitleID
    int LoadCodesForCurrentTitle();

    
};