#include "command_handler.h"

//Yes this is all it adds, these layers are layers for a reason ok?
class GeckoProcessor: public CommandHandler {
public:
    bool geckoSetUp = false;
    //bool isOnline = true;
    bool shouldTakeScreenShot = false;
    bool isCodeHandlerInstalled = false;
    bool areNotificationsOn = true;

    int processCommands();

    int runProcessServer();
};