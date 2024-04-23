#ifndef CODE_HANDLER_H
#define CODE_HANDLER_H

#include <coreinit/thread.h>

//This will likely be changed into an address pointer
#define CODE_HANDLER_INSTALL_ADDRESS 0x010F4000


class CodeHandler {
private:
    //*mumble mumble* array of codes here

    uint32_t assemblySize = 0;


public:

    uint32_t code_count;

    bool running;

    OSThread *thread;

    void executeAssembly();

    void loadSDCodes();

    void run();
};


#endif