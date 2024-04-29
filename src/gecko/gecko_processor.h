#ifndef GECKO_PROCESSOR_H
#define GECKO_PROCESSOR_H

#include <coreinit/thread.h>

#include "command_handler.h"

class GeckoProcessor: public CommandHandler {
public:

    bool running = false;

    OSThread *thread;

    int processCommands();

    static int runGeckoServer(int argc, const char **argv){
        ((GeckoProcessor *) argv)->run();
        
    }

    int run();

};

#endif