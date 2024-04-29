#ifndef CODE_STORAGE_H
#define CODE_STORAGE_H

#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config.h>

class aRAMaConfig {
public:
    //The raw bitfield of settings fetched from and saved to WUPS storage.
    static int32_t settings;
    //A 2nd bitfield, representing values changed in real time in the config menu
    //If it matches settings, aRAMa will not re-init/de-init
    static int32_t settings_live_cache;

    enum ARAMA_SETTINGS {
	ACTIVE = 1 << 0,
	SD_CODES_ACTIVE = 1 << 1,
	NOTIFICATIONS_ON = 1 << 2,
	CODE_HANDLER = 1 << 3,
	AUTO_STORE_CODES = 1 << 4,
	ENABLE_CAFFIINE = 1 << 5,
	ENABLE_SAVIINE = 1 << 6
    };

    static const char active_id;
    static const char sd_codes_id;
    static const char notifs_id;
    static const char code_hand_id;
    static const char autosave_id;
    static const char caffiine_id;
    static const char saviine_id;

    //IP address of the Wii U
    static uint32_t ip_address;

    //Amount of SD codes detected for the currently running title
    static uint32_t local_code_amount;

    //Whether aRAMa as a whole is active or not.
    static bool active;
    //Depreciate? Whether or not SD codes are active.
    static bool sd_codes;
    //TODO: When enabled, aRAMa will notify about running status messages,
    //such as errors, transmissions sent/recieved, etc.
    static bool notifications_enabled;
    //If enabled, codes sent over will be saved to SD card automatically
    static bool auto_save;

    //if tcpgecko is active
    static bool tcpgecko;
    //if caffiine is active
    static bool caffiine;
    //if saviine is active
    static bool saviine;
    //if the code handler is active
    static bool code_handler;

    static void settings_changed(ConfigItemBoolean *item, bool new_value);

    //Create/fetch all the settings in WUPS storage
    static void LoadSettings();
    //Save the settings to WUPS storage
    static void SaveSettings();

};


#endif