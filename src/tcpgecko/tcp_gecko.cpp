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
#include "stack.h"
#include "pause.h"
#include "sd_ip_reader.h"
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
#include "command_handling/shared_functions.h"
#include "command_handling/command_handler.h"

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

#define ERROR_BUFFER_SIZE 150

static int processCommands(struct pygecko_bss_t *bss, int clientfd) {

	CommandHandler handler;
	handler.clientfd = clientfd;
	handler.bss = bss;

	// Run the RPC server
	while (true) {
		handler.checkbyte();
		if (handler.ret < 0) {
			CHECK_ERROR(errno2 != E_WOULD_BLOCK);
			GX2WaitForVsync();
			continue;
		}
		switch (handler.ret) {
			case COMMAND_WRITE_8: {
				handler.command_write_8();
				break;
			}
			case COMMAND_WRITE_16: {
				handler.command_write_16();
				break;
			}
			case COMMAND_WRITE_32: {
				handler.command_write_32();
				break;
			}
			case COMMAND_READ_MEMORY: {
				handler.command_read_memory();
				break;
			}
			case COMMAND_READ_MEMORY_KERNEL: {
				handler.command_read_memory_kernel();
				break;
			}
			case COMMAND_VALIDATE_ADDRESS_RANGE: {
				handler.command_validate_address_range();
				break;
			}
			/*case COMMAND_DISASSEMBLE_RANGE: {
				handler.command_disassemble_range();
				break;
			}*/
			case COMMAND_MEMORY_DISASSEMBLE: {
				handler.command_memory_disassemble();
				break;
			}
				case COMMAND_READ_MEMORY_COMPRESSED: {
				handler.command_read_memory_compressed();
				break;
			}
			case COMMAND_KERNEL_WRITE: {
				handler.command_kernel_write();
				break;
			}
			case COMMAND_KERNEL_READ: {
				handler.command_kernel_read();
				break;
			}
			case COMMAND_TAKE_SCREEN_SHOT: {
				handler.command_take_screenshot();
				break;
			}
			case COMMAND_UPLOAD_MEMORY: {
				handler.command_upload_memory();
				break;
			}
			case COMMAND_GET_DATA_BUFFER_SIZE: {
				handler.command_get_data_buffer_size();
				break;
			}
			case COMMAND_READ_FILE: {
				handler.command_read_file();
				break;
			}
			case COMMAND_READ_DIRECTORY: {
				handler.command_read_directory();
				break;
			}
			case COMMAND_REPLACE_FILE: {
				handler.command_replace_file();
				break;
			}
			case COMMAND_IOSU_HAX_READ_FILE: {
				handler.command_iosu_hax_read_file();
				break;
			}
			case COMMAND_GET_VERSION_HASH: {
				handler.command_get_version_hash();
				break;
			}
			case COMMAND_GET_CODE_HANDLER_ADDRESS: {
				handler.command_get_code_handler_address();
				break;
			}
			case COMMAND_READ_THREADS: {
				handler.command_read_threads();
				break;
			}
			case COMMAND_ACCOUNT_IDENTIFIER: {
				handler.command_account_identifier();
				break;
			}
			/*case COMMAND_WRITE_SCREEN: {
				handler.command_write_screen();
			}*/
			case COMMAND_FOLLOW_POINTER: {
				handler.command_follow_pointer();
				break;
			}
			case COMMAND_SERVER_STATUS: {
				handler.command_server_status();
				break;
			}
			case COMMAND_REMOTE_PROCEDURE_CALL: {
				handler.command_remote_procedure_call();
				break;
			}
			case COMMAND_GET_SYMBOL: {
				handler.command_get_symbol();
				break;
			}
			case COMMAND_MEMORY_SEARCH_32: {
				handler.command_memory_search_32();
				break;
			}
			case COMMAND_ADVANCED_MEMORY_SEARCH: {
				handler.command_advanced_memory_search();
				break;
			}
			case COMMAND_EXECUTE_ASSEMBLY: {
				handler.command_execute_assembly();
				break;
			}
			case COMMAND_PAUSE_CONSOLE: {
				handler.command_pause_console();
				break;
			}
			case COMMAND_RESUME_CONSOLE: {
				handler.command_resume_console();
				break;
			}
			case COMMAND_IS_CONSOLE_PAUSED: {
				handler.command_is_console_paused();
				break;
			}
			case COMMAND_SERVER_VERSION: {
				handler.command_server_version();
				break;
			}
			case COMMAND_GET_OS_VERSION: {
				handler.command_get_os_version();
				break;
			}
			case COMMAND_SET_DATA_BREAKPOINT: {
				handler.command_set_data_breakpoint();
				break;
			}
			case COMMAND_SET_INSTRUCTION_BREAKPOINT: {
				handler.command_set_instruction_breakpoint();
				break;
			}
			case COMMAND_TOGGLE_BREAKPOINT: {
				handler.command_toggle_breakpoint();
				break;
			}
			case COMMAND_REMOVE_ALL_BREAKPOINTS: {
				handler.command_remove_all_breakpoints();
				break;
			}
			case COMMAND_GET_STACK_TRACE: {
				handler.command_get_stack_trace();
				break;
			}
			case COMMAND_POKE_REGISTERS: {
				handler.command_poke_registers();
				break;
			}
			case COMMAND_GET_ENTRY_POINT_ADDRESS: {
				handler.commmand_get_entry_point_address();
				break;
			}
			case COMMAND_RUN_KERNEL_COPY_SERVICE: {
				handler.command_run_kernel_copy_service();
				break;
			}
			case COMMAND_PERSIST_ASSEMBLY: {
				handler.command_persist_assembly();
				break;
			}
			case COMMAND_CLEAR_ASSEMBLY: {
				handler.command_clear_assembly();
				break;
			}
			default: {
				reportIllegalCommandByte(ret);
				break;
			}
		}
	}

	error:
	bss->error = ret;
	return 0;
}

int sockfd = -1, clientfd = -1, ret = 0, len;
struct sockaddr_in socketAddress;
struct pygecko_bss_t *bss;

static int runTCPGeckoServer(int argc, void *argv) {
	bss = (struct pygecko_bss_t *) argv;

	setup_os_exceptions();
	//socket_lib_init();
	initializeUDPLog();

	while (true) {
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = 7331;
		socketAddress.sin_addr.s_addr = 0;

		log_printf("socket()...\n");
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CHECK_ERROR(sockfd == -1)

		log_printf("bind()...\n");
		ret = bind(sockfd, (struct sockaddr *) &socketAddress, (s32) 16);
		CHECK_ERROR(ret < 0)

		log_printf("listen()...\n");
		ret = listen(sockfd, (s32) 20);
		CHECK_ERROR(ret < 0)

		while (true) {
			len = 16;
			log_printf("before accept()...\n");
			clientfd = accept(sockfd, (struct sockaddr *) &socketAddress, (s32 * ) &len);
			log_printf("after accept()...\n");
			CHECK_ERROR(clientfd == -1)
			log_printf("commands()...\n");
			ret = processCommands(bss, clientfd);
			CHECK_ERROR(ret < 0)
			socketclose(clientfd);
			clientfd = -1;

			log_printf("GX2WaitForVsync() inner...\n");
			GX2WaitForVsync();
		}

		error:
		log_printf("error, closing connection...\n");
		if (clientfd != -1)
			socketclose(clientfd);
		if (sockfd != -1)
			socketclose(sockfd);
		bss->error = ret;

		// Fix the console freezing when e.g. going to the friend list
		log_printf("GX2WaitForVsync() outer...\n");
		GX2WaitForVsync();
	}
	return 0;
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

	int status = (int) OSCreateThread(thread, startTCPGeckoThread, (s32) 1,
								NULL, (s32)(stack + sizeof(stack)),
								sizeof(stack), 0,
								(OS_THREAD_ATTRIB_AFFINITY_CPU1 | (OSThreadAttributes) OS_THREAD_ATTR_PINNED_AFFINITY |
								 OS_THREAD_ATTRIB_DETACHED));
	ASSERT_INTEGER(status, 1, "Creating TCP Gecko thread")
	// OSSetThreadName(thread, "TCP Gecko");
	OSResumeThread(thread);

	log_print("TCP Gecko started...\n");
}