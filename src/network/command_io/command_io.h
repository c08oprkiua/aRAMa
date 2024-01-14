#include <coreinit/thread.h>
#include <coreinit/filesystem.h>

#include "../net_common.h"

#define SERVER_VERSION "06/03/2017"

#define MSG_DONT_WAIT 32

#define FS_CLIENT_SIZE                  0x1700
#define FS_CMD_BLOCK_SIZE               0xA80

#define ERROR_BUFFER_SIZE 150
#define DATA_BUFFER_SIZE 0x5000

#define CHECK_ERROR(cond)     \
	if (cond)                 \
	{                         \
		line = __LINE__; \
		goto error;       \
	}

FSClient *client;
FSCmdBlock *commandBlock;
bool kernelCopyServiceStarted;

enum code_mode{
    CODE_SD_CARD,
    CODE_TCP,
    CODE_USB, // Future plans
    CODE_OTHER, //Just in case
}

class CommandIO
{
public:

    //Tells the backing code where to point to for retrieving codes
    code_mode mode = CODE_SD_CARD;

    OSThread *thread;
    
    //ret must be signed because it is set to -1 for errors, idk if any of the others 
    //share a similar requirement to be signed, or to be large numbers
    int32_t ret;
	uint32_t error, line, clientfd, sock;

    unsigned char stack[0x6F00];

	unsigned char buffer[0x5001];

    int getMode(int *result);

    int checkbyte();

    int recvwait(int len);
    int recvwait_buffer(unsigned char *buffer, int len);

    int recvwaitlen(int len);
    int recvwaitlen_buffer(void *buffer, int len);

    int recvbyte();

    int sendwait(int len);
    int sendwait_buffer(unsigned char *buffer, int len);

    int sendByte(u_char byte);

    unsigned int receiveString(unsigned char *stringBuffer, unsigned int bufferSize);

    void log_string(const char *str, char flag_byte);
};