#include <coreinit/thread.h>
#include <coreinit/filesystem.h>

#define SERVER_VERSION "06/03/2017"

#define MSG_DONT_WAIT 32

#define FS_CLIENT_SIZE                  0x1700
#define FS_CMD_BLOCK_SIZE               0xA80

#define ERROR_BUFFER_SIZE 150
#define DATA_BUFFER_SIZE 0x5000


//void *client;
FSClient *client;
//void *commandBlock;
FSCmdBlock *commandBlock;
bool kernelCopyServiceStarted;

struct pygecko_bss_t {
	int error, line;
	//void *thread;
	OSThread *thread;
	unsigned char stack[0x6F00];
};

class CommandIO {
public:

	int ret;
	pygecko_bss_t *bss;
	int clientfd;
    int sock;

	unsigned char buffer[0x5001];

    void check_err(bool cond);

    int recvwait(int len);

    int recvwait_buffer(unsigned char *buffer, int len);

    int recvbyte();

    int checkbyte();

    int sendwait(int len);

    int sendwait_buffer(unsigned char *buffer, int len);

    int sendByte(u_char byte);

    unsigned int receiveString(unsigned char *stringBuffer, unsigned int bufferSize);

};