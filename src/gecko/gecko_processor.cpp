#include "tcpgecko/tcp_gecko_commands.h"
#include "gecko_processor.h"

#include <nsysnet/socket.h>
#include <gx2/event.h>

#include <newlib.h>
#include <coreinit/internal.h>
#include <whb/log.h>


int GeckoProcessor::processCommands() {
	// Run the RPC server
	while (true) {
	    ret = checkbyte();
		if (ret < 0) {
			CHECK_ERROR(errno2 != E_WOULD_BLOCK);
			GX2WaitForVsync();
			continue;
		}
		//Store code here

		switch (ret) {
			case COMMAND_WRITE_8: {
				command_write_8();
				break;
			}
			case COMMAND_WRITE_16: {
				command_write_16();
				break;
			}
			case COMMAND_WRITE_32: {
				command_write_32();
				break;
			}
			case COMMAND_READ_MEMORY: {
				command_read_memory();
				break;
			}
			case COMMAND_READ_MEMORY_KERNEL: {
				command_read_memory_kernel();
				break;
			}
			case COMMAND_VALIDATE_ADDRESS_RANGE: {
				command_validate_address_range();
				break;
			}
			/*case COMMAND_DISASSEMBLE_RANGE: {
				command_disassemble_range();
				break;
			}*/
			case COMMAND_MEMORY_DISASSEMBLE: {
				command_memory_disassemble();
				break;
			}
				case COMMAND_READ_MEMORY_COMPRESSED: {
				command_read_memory_compressed();
				break;
			}
			case COMMAND_KERNEL_WRITE: {
				command_kernel_write();
				break;
			}
			case COMMAND_KERNEL_READ: {
				command_kernel_read();
				break;
			}
			case COMMAND_TAKE_SCREEN_SHOT: {
				command_take_screenshot();
				break;
			}
			case COMMAND_UPLOAD_MEMORY: {
				command_upload_memory();
				break;
			}
			case COMMAND_GET_DATA_BUFFER_SIZE: {
				command_get_data_buffer_size();
				break;
			}
			case COMMAND_READ_FILE: {
				command_read_file();
				break;
			}
			case COMMAND_READ_DIRECTORY: {
				command_read_directory();
				break;
			}
			case COMMAND_REPLACE_FILE: {
				command_replace_file();
				break;
			}
			case COMMAND_IOSU_HAX_READ_FILE: {
				command_iosu_hax_read_file();
				break;
			}
			case COMMAND_GET_VERSION_HASH: {
				command_get_version_hash();
				break;
			}
			case COMMAND_GET_CODE_HANDLER_ADDRESS: {
				command_get_code_handler_address();
				break;
			}
			case COMMAND_READ_THREADS: {
				command_read_threads();
				break;
			}
			case COMMAND_ACCOUNT_IDENTIFIER: {
				command_account_identifier();
				break;
			}
			/*case COMMAND_WRITE_SCREEN: {
				command_write_screen();
			}*/
			case COMMAND_FOLLOW_POINTER: {
				command_follow_pointer();
				break;
			}
			case COMMAND_SERVER_STATUS: {
				command_server_status();
				break;
			}
			case COMMAND_REMOTE_PROCEDURE_CALL: {
				command_remote_procedure_call();
				break;
			}
			case COMMAND_GET_SYMBOL: {
				command_get_symbol();
				break;
			}
			case COMMAND_MEMORY_SEARCH_32: {
				command_memory_search_32();
				break;
			}
			case COMMAND_ADVANCED_MEMORY_SEARCH: {
				command_advanced_memory_search();
				break;
			}
			case COMMAND_EXECUTE_ASSEMBLY: {
				command_execute_assembly();
				break;
			}
			case COMMAND_PAUSE_CONSOLE: {
				command_pause_console();
				break;
			}
			case COMMAND_RESUME_CONSOLE: {
				command_resume_console();
				break;
			}
			case COMMAND_IS_CONSOLE_PAUSED: {
				command_is_console_paused();
				break;
			}
			case COMMAND_SERVER_VERSION: {
				command_server_version();
				break;
			}
			case COMMAND_GET_OS_VERSION: {
				command_get_os_version();
				break;
			}
			case COMMAND_SET_DATA_BREAKPOINT: {
				command_set_data_breakpoint();
				break;
			}
			case COMMAND_SET_INSTRUCTION_BREAKPOINT: {
				command_set_instruction_breakpoint();
				break;
			}
			case COMMAND_TOGGLE_BREAKPOINT: {
				command_toggle_breakpoint();
				break;
			}
			case COMMAND_REMOVE_ALL_BREAKPOINTS: {
				command_remove_all_breakpoints();
				break;
			}
			case COMMAND_GET_STACK_TRACE: {
				command_get_stack_trace();
				break;
			}
			case COMMAND_POKE_REGISTERS: {
				command_poke_registers();
				break;
			}
			case COMMAND_GET_ENTRY_POINT_ADDRESS: {
				commmand_get_entry_point_address();
				break;
			}
			case COMMAND_RUN_KERNEL_COPY_SERVICE: {
				command_run_kernel_copy_service();
				break;
			}
			case COMMAND_PERSIST_ASSEMBLY: {
				command_persist_assembly();
				break;
			}
			case COMMAND_CLEAR_ASSEMBLY: {
				command_clear_assembly();
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

int GeckoProcessor::runProcessServer(){

	struct sockaddr_in socketAddress;

	int sockfd = -1, len;

	while (true) {
		socketAddress.sin_family = AF_INET;
		socketAddress.sin_port = 7331;
		socketAddress.sin_addr.s_addr = 0;

		WHBLogPrintf("socket()...\n");
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CHECK_ERROR(sockfd == -1)

		WHBLogPrintf("bind()...\n");
		ret = bind(sockfd, (struct sockaddr *) &socketAddress, (socklen_t) 16);
		CHECK_ERROR(ret < 0)

		WHBLogPrintf("listen()...\n");
		ret = listen(sockfd, (int) 20);
		CHECK_ERROR(ret < 0)

		while (true) {
			len = 16;
			WHBLogPrintf("before accept()...\n");
			clientfd = accept(sockfd, (struct sockaddr *) &socketAddress, (socklen_t *) &len);
			WHBLogPrintf("after accept()...\n");
			CHECK_ERROR(clientfd == -1)
			WHBLogPrintf("commands()...\n");
			ret = processCommands();
			CHECK_ERROR(ret < 0)
			socketclose(clientfd);
			clientfd = -1;

			WHBLogPrintf("GX2WaitForVsync() inner...\n");
			GX2WaitForVsync();
		}

		error:
		WHBLogPrintf("error, closing connection...\n");
		if (clientfd != -1)
			socketclose(clientfd);
		if (sockfd != -1)
			socketclose(sockfd);
		error = ret;

		// Fix the console freezing when e.g. going to the friend list
		WHBLogPrintf("GX2WaitForVsync() outer...\n");
		GX2WaitForVsync();
	}
	return 0;
}

