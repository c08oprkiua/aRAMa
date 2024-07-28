#include "thread_server.h"
#include <whb/log.h>

bool aRAMaServer::is_static_init = false;

static int runServer(int argc, const char **argv){
    return ((aRAMaServer *) argv)->run();
}

aRAMaServer::aRAMaServer(uint32_t stack_size){
    if (!is_static_init){
        OSInitMutex(&fsMutex);
        is_static_init = true;
    }
    //alloc stack
}

aRAMaServer::~aRAMaServer(){
    stop_server();
    //unalloc stack
}

int aRAMaServer::start_server(OSThreadAttributes attributes){
	WHBLogPrint("aRAMa: starting server thread\n");
    running = true;

	if (OSCreateThread(
	thread, runServer, 
	1, (char *) this,
	(stack + sizeof(stack)), sizeof(stack), 
	0, attributes
    ) == true) {
	    OSResumeThread(thread);
	}

	WHBLogPrint("aRAMa: server started\n");
}

int aRAMaServer::stop_server(){
    running = false;
    int thread_result;
    OSJoinThread(thread, &thread_result);
    return thread_result;
}