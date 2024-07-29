#include "../command_handler.h"
#include "coreinit/thread.h"
#include <kernel/kernel.h>

#define KERNEL_COPY_SOURCE_ADDRESS 0x10100000

OSThread kernel_thread;

static bool kernelCopyServiceStarted;

int kernelCopyService(int argc, const char **argv) {
	while (true) {
		// Read the destination address from the source address
		uint32_t destinationAddress = *(int *) KERNEL_COPY_SOURCE_ADDRESS;

		// Avoid crashing
		if (OSIsAddressValid(destinationAddress)) {
			// Perform memory copy
			uint32_t *valueBuffer = (uint32_t *) (KERNEL_COPY_SOURCE_ADDRESS + 4);
			KernelCopyData(destinationAddress, *valueBuffer, 4);

			// "Consume" address and value for synchronization with the code handler for instance
			*(int *) KERNEL_COPY_SOURCE_ADDRESS = 0;
			*(((int *) KERNEL_COPY_SOURCE_ADDRESS) + 1) = 0;
		}
	}

	// This is here to avoid the warning
	return 0;
}

void startKernelCopyService() {
	uint32_t stack = (uint32_t) memalign(0x40, 0x100);

	if (stack != 0) {
		stack += 0x100;

		BOOL status = OSCreateThread(&kernel_thread, kernelCopyService, 1, NULL, (void *) stack + sizeof(stack), sizeof(stack), 31,
									OS_THREAD_ATTRIB_AFFINITY_CPU1 | /*OS_THREAD_ATTR_PINNED_AFFINITY |*/ OS_THREAD_ATTRIB_DETACHED);
		ASSERT_INTEGER(status, TRUE, "Creating kernel copy thread")
		OSSetThreadName(&kernel_thread, "aRAMa Kernel Copier");
		OSResumeThread(&kernel_thread);
	}
}
void CommandHandler::command_kernel_write(){
	ret = recvwait(sizeof(int) * 2);
	uint32_t address, value;

	CHECK_ERROR(ret < 0);

	address = ((uint32_t *)buffer)[0];
	value = ((uint32_t *)buffer)[1];

	KernelCopyData(address, value, sizeof(value));

	error:
	error = ret;
	return;
};

void CommandHandler::command_kernel_read(){
	ret = recvwait(sizeof(int));
	void *address, *value;
	CHECK_ERROR(ret < 0);

	address = ((void **)buffer)[0];
	value = (void *)readKernelMemory(address);

	*(void **)buffer = value;
	sendwait(sizeof(int));

	error:
	error = ret;
	return;
};

void CommandHandler::command_run_kernel_copy_service(){
	if (!kernelCopyServiceStarted){
		kernelCopyServiceStarted = true;
		startKernelCopyService();
	}
};