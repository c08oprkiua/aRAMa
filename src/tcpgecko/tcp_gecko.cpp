#include "tcp_gecko.h"
// #include <iosuhax.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

// #include <inttypes.h>
#include "../common/common.h"
//#include <zlib.h> // Actually must be included before os_functions

#include "main.h"

//#include "../dynamic_libs/fs_functions.h"
#include <coreinit/filesystem.h>

#include "../utils/logger.h"
#include "hardware_breakpoints.h"
#include "linked_list.h"
#include "address.h"
//#include "stack.h"
#include "pause.h"
//#include "sd_ip_reader.h"
//#include "../patcher/function_patcher_gx2.h"
//#include "sd_cheats.h"
//#include "threads.h"
//#include "software_breakpoints.h"


//WUT includes
#include <coreinit/thread.h> //replaces os_functions.h
#include <coreinit/memory.h>
#include <nn/act.h>

#include <nsysnet/socket.h> //replaces socket_functions.h

//Regorganization based includes
#include "tcp_gecko_commands.h"
#include "../gecko/gecko_processor.h"

#include "assertions.h"
#include "raw_assembly_cheats.h"


#include <gx2/event.h>


#define CHECK_ERROR(cond) if (cond) { bss->line = __LINE__; goto error; }

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
	log_print("Starting TCP Gecko thread...\n");

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

	log_print("TCP Gecko thread started...\n");

	// Execute the code handler if it is installed
	if (isCodeHandlerInstalled) {
		log_print("Code handler installed...\n");
		void (*codeHandlerFunction)() = (void (*)()) CODE_HANDLER_INSTALL_ADDRESS;

		while (true) {
			usleep(9000);

			// considerApplyingSDCheats();
			// log_print("Running code handler...\n");
			codeHandlerFunction();
			// log_print("Code handler done executing...\n");

			if (assemblySize > 0) {
				executeAssembly();
			}

			if (areSDCheatsEnabled) {
				considerApplyingSDCheats();
			}
		}
	} else {
		log_print("Code handler not installed...\n");
	}

	return (s32) 0;
}

void startTCPGecko() {
	log_print("Starting TCP Gecko...\n");

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
		(OS_THREAD_ATTRIB_AFFINITY_CPU1 /*| (OSThreadAttributes) OS_THREAD_ATTR_PINNED_AFFINITY*/ |OS_THREAD_ATTRIB_DETACHED));
	ASSERT_INTEGER(status, 1, "Creating TCP Gecko thread")
	// OSSetThreadName(thread, "TCP Gecko");
	OSResumeThread(thread);

	log_print("TCP Gecko started...\n");
}