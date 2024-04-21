#ifndef CODE_HANDLER_H
#define CODE_HANDLER_H

#include <coreinit/thread.h>

class CodeHandler {
private:

    void executeAssembly();

    void loadSDCodes();
public:
    bool running;

    OSThread thread;


    void runCodeHandlerServer();
};


#endif