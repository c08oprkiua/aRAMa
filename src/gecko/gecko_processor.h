#ifndef GECKO_PROCESSOR_H
#define GECKO_PROCESSOR_H

#include <coreinit/thread.h>

#include "command_handler.h"

class GeckoProcessor: public CommandHandler {
public:

    int processCommands();

    int run() override;

};

#endif