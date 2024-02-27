#include "gecko/gecko_processor.h"
#include "arama.h"

#include "tcpgecko/raw_assembly_codes.h"
#include "tcpgecko/sd_codes.h"

#include <nsysnet/socket.h>
#include <gx2/event.h>

#define CODE_HANDLER_INSTALL_ADDRESS 0x010F4000

//This is redundant, and is in GeckoProcessor
/*
int runGeckoServer(uint32_t argc, char *argv){
	GeckoProcessor *processor;

    int sockfd = -1, len;
    struct sockaddr_in socketAddress;

	//setup_os_exceptions();
	//socket_lib_init();
	//initializeUDPLog();

    GeckoSetUp = true;

	while (GeckoSetUp) {
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = 7331;
		socketAddress.sin_addr.s_addr = 0;

		WHBLogPrintf("socket()...\n");
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CHECK_ERROR(sockfd == -1);

		WHBLogPrintf("bind()...\n");
		processor->ret = bind(sockfd, (struct sockaddr *) &socketAddress, (socklen_t) 16);
		CHECK_ERROR(processor->ret < 0);

		WHBLogPrintf("listen()...\n");
		processor->ret = listen(sockfd, (int) 20);
		CHECK_ERROR(processor->ret < 0);

		while (true) {
			len = 16;
			WHBLogPrintf("before accept()...\n");
			processor->clientfd = accept(sockfd, (struct sockaddr *) &socketAddress, (socklen_t *) &len);
			WHBLogPrintf("after accept()...\n");
			CHECK_ERROR(processor->clientfd == -1);
			WHBLogPrintf("commands()...\n");
			processor->ret = processor->processCommands();
			CHECK_ERROR(processor->ret < 0);
			socketclose(processor->clientfd);
			processor->clientfd = -1;

			WHBLogPrintf("GX2WaitForVsync() inner...\n");
			GX2WaitForVsync();
		}

		error:
		WHBLogPrintf("error, closing connection...\n");
		if (processor->clientfd != -1)
			socketclose(processor->clientfd);
		if (sockfd != -1)
			socketclose(sockfd);
		processor->error = processor->ret;

		// Fix the console freezing when e.g. going to the friend list
		WHBLogPrintf("GX2WaitForVsync() outer...\n");
		GX2WaitForVsync();
	}
	
	return 0;
}
*/

static int CreateGeckoThread(){
	WHBLogPrint("Starting aRAMa thread. Welcome To The Next Level!\n");
	GeckoProcessor geck_proc;
}

static signed int startTCPGeckoThread(signed int argc, void *argv) {
	WHBLogPrint("Starting TCP Gecko thread...\n");

	// Run the TCP Gecko Installer server
	GeckoProcessor geck_proc;

    /*
	bss = (struct pygecko_bss_t *) memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss == 0)
		return (s32) 0;
	memset(bss, 0, sizeof(struct pygecko_bss_t));
    */

	if (OSCreateThread(
		geck_proc.thread, 
		(OSThreadEntryPointFn) runGeckoServer, 
		(uint32_t) 1, 
		(char *) &geck_proc,
		(void *)geck_proc.stack + sizeof(geck_proc.stack), 
		sizeof(geck_proc.stack), 
		0, 
		0xc
		) 
	== 1) {
		OSResumeThread(geck_proc.thread);
	}

	WHBLogPrint("TCP Gecko thread started...\n");

	// Execute the code handler if it is installed
	if (isCodeHandlerInstalled) {
		WHBLogPrint("Code handler installed...\n");
		void (*codeHandlerFunction)() = (void (*)()) CODE_HANDLER_INSTALL_ADDRESS;

		while (true) {
			usleep(9000);

			// considerApplyingSDCheats();
			// WHBLogPrint("Running code handler...\n");
			codeHandlerFunction();
			// WHBLogPrint("Code handler done executing...\n");

			if (assemblySize > 0) {
				executeAssembly();
			}

			if (arama_settings && ARAMA_SET_SD_CODES_ACTIVE) {
				considerApplyingSDCheats();
			}
		}
	} else {
		WHBLogPrint("Code handler not installed...\n");
	}

	return (signed int) 0;
}

//FROM tcp_gecko.cpp

//#define CHECK_ERROR(cond) if (cond) { bss->line = __LINE__; goto error; }

/*ZEXTERN int ZEXPORT
deflateEnd OF((z_streamp
strm));
ZEXTERN int ZEXPORT
deflateInit OF((z_streamp
strm,
int level
));
ZEXTERN int ZEXPORT
deflate OF((z_streamp
strm,
int flush
));*/

// ########## Being socket_functions.h ############

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

/*

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

static s32 startTCPGeckoThread(s32 argc, void *argv) {
	WHBLogPrint("Starting TCP Gecko thread...\n");

	// Run the TCP Gecko Installer server
	struct pygecko_bss_t *bss;

	bss = (struct pygecko_bss_t *) memalign(0x40, sizeof(struct pygecko_bss_t));
	if (bss == 0)
		return (s32) 0;
	memset(bss, 0, sizeof(struct pygecko_bss_t));

	if (OSCreateThread(&bss->thread, (s32 (*)(s32, void *)) runTCPGeckoServer, 1, bss,
					   (u32) bss->stack + sizeof(bss->stack),
					   sizeof(bss->stack), 0,
					   0xc) == 1) {
		OSResumeThread(bss->thread);
	} else {
		free(bss);
	}

	WHBLogPrint("TCP Gecko thread started...\n");

	// Execute the code handler if it is installed
	if (isCodeHandlerInstalled) {
		WHBLogPrint("Code handler installed...\n");
		void (*codeHandlerFunction)() = (void (*)()) CODE_HANDLER_INSTALL_ADDRESS;

		while (true) {
			usleep(9000);

			// considerApplyingSDCheats();
			// WHBLogPrint("Running code handler...\n");
			codeHandlerFunction();
			// WHBLogPrint("Code handler done executing...\n");

			if (assemblySize > 0) {
				executeAssembly();
			}

			if (areSDCheatsEnabled) {
				considerApplyingSDCheats();
			}
		}
	} else {
		WHBLogPrint("Code handler not installed...\n");
	}

	return (s32) 0;
}

void startTCPGecko() {
	WHBLogPrint("Starting TCP Gecko...\n");

	// Force the debugger to be initialized by default
	// writeInt((unsigned int) (OSIsDebuggerInitialized + 0x1C), 0x38000001); // li r3, 1

	unsigned int stack = (unsigned int) memalign(0x40, 0x100);
	ASSERT_ALLOCATED(stack, "TCP Gecko stack")
	stack += 0x100;
	//void *thread = memalign(0x40, 0x1000);
	OSThread *thread;
	ASSERT_ALLOCATED(thread, "TCP Gecko thread")

	int status = (int) OSCreateThread(
		thread, 
		startTCPGeckoThread, 
		(s32) 1,
		NULL, 
		(s32)(stack + sizeof(stack)),
		sizeof(stack), 0,
		(OS_THREAD_ATTRIB_AFFINITY_CPU1 | (OSThreadAttributes) OS_THREAD_ATTR_PINNED_AFFINITY |OS_THREAD_ATTRIB_DETACHED));
	ASSERT_INTEGER(status, 1, "Creating TCP Gecko thread")
	// OSSetThreadName(thread, "TCP Gecko");
	OSResumeThread(thread);

	WHBLogPrint("TCP Gecko started...\n");
}

*/