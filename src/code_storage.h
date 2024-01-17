#include <wups/storage.h>
#include <wups.h>

#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config.h>

//Create all the settings in WUPS storage
void InitaRAMaSettings();
void SaveaRAMaSettings();
//Get codes based on the TitleID
void GetCodesByTitle();

void StoreSentCode();

class aRAMaConfig {
public:

    WUPSConfigHandle base;
    WUPSConfigCategoryHandle arama_category;
    WUPSConfigCategoryHandle codes_category;



    void LoadBaseConfigMenu();
    void LoadCodesForCurrentTitle();

    
};