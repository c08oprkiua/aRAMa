#ifndef ARAMA_H
#define ARAMA_H

#include "gecko/gecko_processor.h"

#define ARAMA_SET_ACTIVE 1 << 0
#define ARAMA_SET_SD_CODES_ACTIVE 1 << 1
#define ARAMA_SET_NOTIFICATIONS_ON 1 << 2
#define ARAMA_SET_NO_ONLINE 1 << 3
#define ARAMA_SET_AUTO_STORE_CODES 1 << 4
#define ARAMA_ENABLE_CAFFIINE 1 << 5
#define ARAMA_ENABLE_SAVIINE 1 << 6

enum ARAMA_SETTINGS {
	ACTIVE = 1 << 0,
	SD_CODES_ACTIVE = 1 << 1,
	NOTIFICATIONS_ON = 1 << 2,
	NO_ONLINE = 1 << 3,
	AUTO_STORE_CODES = 1 << 4,
	ENABLE_CAFFIINE = 1 << 5,
	ENABLE_SAVIINE = 1 << 6
};

//This is a bitfield of STORED values
//It has to be uint32_t for WUPS storage
uint32_t arama_settings = (ARAMA_SET_ACTIVE | ARAMA_SET_NOTIFICATIONS_ON);

//Todo: Get IP from console, display in WUPS menu a la FTPiiU
uint32_t ip_address;

bool geckoSetUp = false;
bool isOnline = true;
bool isCodeHandlerInstalled = false;
bool areNotificationsOn = true;


static int CreateGeckoThread();
static signed int runGeckoServer(GeckoProcessor *processor);

#endif