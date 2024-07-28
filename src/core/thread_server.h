#ifndef ARAMA_SERVER_H
#define ARAMA_SERVER_H

#include <coreinit/thread.h>
#include <coreinit/mutex.h>
#include <coreinit/filesystem.h>

#include "log_and_noti.h"

//Based on CThread, from FTPiiU and other HB apps/plugins

//A base class for managing aRAMa's various servers
class aRAMaServer {
private:
    static bool is_static_init;

    uint8_t *stack;

public:
    aRAMaServer(uint32_t stack_size = 0x6F00);
    ~aRAMaServer();

    //if the server is running. Is used in overrides of run()
    //as the "while true" value, and is set to true/false in some scenarios.
    bool running;

    //Mutex for filesystem stuff
    static OSMutex fsMutex;

    //FSClient for filesystem stuff
    static FSClient *fsCli;

    static FSCmdBlock *fsCommandBlock;

    //The thread for this server to run on
    OSThread *thread;

    virtual int run();

    int start_server(OSThreadAttributes attributes = (OS_THREAD_ATTRIB_AFFINITY_CPU2 | OS_THREAD_ATTRIB_DETACHED));

    int stop_server();
};

#endif
