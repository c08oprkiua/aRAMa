
//Screenshot variables
static volatile int executionCounter = 0;

/*This is a bitfield:
    Arama is active: 1 << 0 
    SD codes are active: 1 << 1
    Notifications are active: 1 << 2
    Operate offline: 1 << 3
*/
uint8_t arama_settings = 0;


//These might be unneeded here, so they may move to screenshot.cpp
unsigned int remainingImageSize = 0;
unsigned int totalImageSize = 0;
int bufferedImageSize = 0;
void *bufferedImageData = nullptr;