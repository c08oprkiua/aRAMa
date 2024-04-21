#ifndef CODE_STORAGE_H
#define CODE_STORAGE_H

#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config.h>

void setting_changed(ConfigItemBoolean* item, bool new_value);

class aRAMaConfig {
private:
    //The raw bitfield of settings fetched from and saved to WUPS storage.
    int32_t settings;

    //The base.
    WUPSConfigHandle base;
    //Category for aRAMa's settings
    WUPSConfigCategoryHandle arama_category;
    //Category for stored codes for the current titleID
    WUPSConfigCategoryHandle codes_category;

    enum ARAMA_SETTINGS {
	ACTIVE = 1 << 0,
	SD_CODES_ACTIVE = 1 << 1,
	NOTIFICATIONS_ON = 1 << 2,
	NO_ONLINE = 1 << 3,
	AUTO_STORE_CODES = 1 << 4,
	ENABLE_CAFFIINE = 1 << 5,
	ENABLE_SAVIINE = 1 << 6
    };

public:
    //IP address of the Wii U
    uint32_t ip_address;

    //Whether aRAMa as a whole is active or not.
    bool active;
    //Depreciate? Whether or not SD codes are active.
    bool sd_codes;
    //TODO: When enabled, aRAMa will notify about running status messages,
    //such as errors, transmissions sent/recieved, etc.
    bool notifications_enabled;
    //DEPRECIATE: when enabled, aRAMa will not connect to TCP.
    bool no_online;
    //If enabled, codes sent over will be saved to SD card automatically
    bool auto_save;

    //if tcpgecko is active
    bool tcpgecko;
    //if caffiine is active
    bool caffiine;
    //if saviine is active
    bool saviine;
    //if the code handler is active
    bool code_handler;

    //Create/fetch all the settings in WUPS storage
    void LoadSettings();
    //Save the settings to WUPS storage
    void SaveSettings();

    //Load the base settings menu, ie. what's loaded from storage.
    int LoadBaseConfigMenu();
    //TODO: Load a second menu listing codes for the current titleID a la SDCaffiine
    int LoadCodesForCurrentTitle();
    
};

#endif 