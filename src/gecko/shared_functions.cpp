#include "../tcpgecko/assertions.h"

#include <stdlib.h>

#include <coreinit/debug.h>
#include <coreinit/filesystem.h>
#include <nsysnet/socket.h>

#define ERROR_BUFFER_SIZE 150

// ########## End socket_functions.h ############

/*void writeScreen(char message[100], int secondsDelay) {
	// TODO Does nothing then crashes (in games)?
	OSScreenClearBufferEx(0, 0);
	OSScreenClearBufferEx(1, 0);

	OSScreenPutFontEx(0, 14, 1, message);
	OSScreenPutFontEx(1, 14, 1, message);

	sleep(secondsDelay);

	OSScreenFlipBuffersEx(0);
	OSScreenFlipBuffersEx(1);
}*/


/* TODO
 		https://github.com/dimok789/ddd/blob/ca33ad1c759a0b67db33eedcf7fc4537198aad9c/src/discdumper.c#L667-L693
		int ret = FSBindMount(pClient, pCmd, metaDir, "/vol/meta", -1);
		FSBindUnmount(pClient, pCmd, metaDir, -1);
	*/
void considerInitializingFileSystem() {
	if (!client) {
		// Initialize the file system
		//int status = FSInit();
		FSStatus status;
		FSInit();
		//ASSERT_FUNCTION_SUCCEEDED(status, "FSInit")

		// Allocate the client
		client = malloc(FS_CLIENT_SIZE);
		ASSERT_ALLOCATED(client, "Client");

		// Register the client
		status = FSAddClientEx(client, 0, -1);
		ASSERT_FUNCTION_SUCCEEDED(status, "FSAddClientEx")

		// Allocate the command block
		commandBlock = malloc(FS_CMD_BLOCK_SIZE);
		ASSERT_ALLOCATED(commandBlock, "Command block")

		FSInitCmdBlock(commandBlock);
	}
}
