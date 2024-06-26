#include "../command_handler.h"
#include <kernel/kernel.h>

void CommandHandler::command_kernel_write(){
	ret = recvwait(sizeof(int) * 2);

	CHECK_ERROR(ret < 0)

	void *address = ((void **)buffer)[0];
	void *value = ((void **)buffer)[1];

	writeKernelMemory(address, (uint32_t)value);
};

void CommandHandler::command_kernel_read(){
	ret = recvwait(sizeof(int));
	if (check_error((ret < 0), FAIL_SOCKET_OTHER, 45)){
		return;
	}
	//CHECK_ERROR(ret < 0)

	void *address = ((void **)buffer)[0];
	void *value = (void *)readKernelMemory(address);

	*(void **)buffer = value;
	sendwait(sizeof(int));
};

void CommandHandler::command_run_kernel_copy_service(){
	if (!kernelCopyServiceStarted){
		kernelCopyServiceStarted = true;
		startKernelCopyService();
	}
};