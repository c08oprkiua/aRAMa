#include "gecko/gecko_processor.h"

#define ARAMA_SET_ACTIVE 1 << 0
#define ARAMA_SET_SD_CODES_ACTIVE 1 << 1
#define ARAMA_SET_NOTIFICATIONS_ON 1 << 2
#define ARAMA_SET_NO_ONLINE 1 << 3
#define ARAMA_SET_AUTO_STORE_CODES 1 << 4
#define ARAMA_ENABLE_CAFFIINE 1 << 5
#define ARAMA_ENABLE_SAVIINE 1 << 6

//This is a bitfield of STORED values
//It has to be uint32_t for WUPS storage
uint32_t arama_settings = (ARAMA_SET_ACTIVE | ARAMA_SET_NOTIFICATIONS_ON);

//Todo: Get IP from console, display in WUPS menu a la FTPiiU
char *ip_address;

bool GeckoSetUp = false;
bool TCPSetUp = false;
bool shouldTakeScreenShot = false;
bool isCodeHandlerInstalled = false;


static int CreateGeckoThread();
static int runGeckoServer(GeckoProcessor *processor);

//Screenshot variables
static volatile int executionCounter = 0;
//These might be unneeded here, so they may move to screenshot.cpp
unsigned int remainingImageSize = 0;
unsigned int totalImageSize = 0;
int bufferedImageSize = 0;
void *bufferedImageData = nullptr;