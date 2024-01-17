#include "command_handler.h"

//Yes this is all it adds, these layers are layers for a reason ok?
class GeckoProcessor: public CommandHandler {
public:

    int processCommands();

    int runProcessServer();
};