#ifndef COMMAND_IO_H
#define COMMAND_IO_H

#include <coreinit/thread.h>
#include <coreinit/mutex.h>
#include <coreinit/filesystem.h>
#include <whb/log.h>

#include "net_common.h"

#define SERVER_VERSION "4/17/2024"

#define MSG_DONT_WAIT 32

#define FS_CLIENT_SIZE 0x1700
#define FS_CMD_BLOCK_SIZE 0xA80

#define ERROR_BUFFER_SIZE 150
#define DATA_BUFFER_SIZE 0x5000


#define CHECK_ERROR(cond)     \
	if (cond)                 \
	{                         \
		line = __LINE__; \
		goto error;       \
}

#define ASSERT_FUNCTION_SUCCEEDED(returnValue, functionName) \
    if (returnValue < 0) { \
        char buffer[100] = {0}; \
        WHBLogPrintf(buffer, 100, "%s failed with return value: %i", functionName, returnValue); \
        OSFatal(buffer); \
} \

static FSClient *client;
static FSCmdBlock *commandBlock;
static bool kernelCopyServiceStarted;

class CommandIO {
public:

    static OSMutex iLock;
    
    //ret must be signed because it is set to -1 for errors, idk if any of the others 
    //share a similar requirement to be signed, or to be large numbers
    int32_t ret;
	uint32_t error, line, clientfd, sock;

    uint8_t stack[0x6F00];

	uint8_t buffer[0x5001];

    int getMode(int *result);

    int checkbyte();

    int recvwait(int len);
    int recvwait_buffer(uint8_t *buffer, int len);

    int recvwaitlen(int len);
    int recvwaitlen_buffer(void *buffer, int len);

    int recvbyte();

    int sendwait(int len);
    int sendwait_buffer(uint8_t *buffer, int len);

    int sendByte(u_char byte);

    uint32_t receiveString(uint8_t *stringBuffer, uint32_t bufferSize);

    void log_string(const char *str, char flag_byte);
};

#endif