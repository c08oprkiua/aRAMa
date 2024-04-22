#ifndef GECKO_PROCESSOR_H
#define GECKO_PROCESSOR_H

#include <coreinit/thread.h>

#include "command_handler.h"

class GeckoProcessor: public CommandHandler {
public:

    GeckoProcessor(bool active = false){
        running = active;
    }

    bool running = false;

    OSThread *thread;

    int processCommands();

    int run();

};

//Another thing taken from FTPiiU
static int runGeckoServer(int argc, const char **argv){
    ((GeckoProcessor *) argv)->run();
    return 0;
}


#endif