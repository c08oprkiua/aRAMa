#include "threads.h"
#include "linked_list.h"

//#include "../dynamic_libs/os_functions.h"
#include <coreinit/thread.h>

//#include "../utils/logger.h"
#include <whb/log.h>


struct node *getAllThreads() {
	struct node *threads = NULL;
	//int currentThreadAddress = OSGetCurrentThread();
	OSThread *currentThreadAddress = OSGetCurrentThread();
	WHBLogPrintf("Thread address: %08x\n", currentThreadAddress);
	//int iterationThreadAddress = currentThreadAddress;
	OSThread *iterationThreadAddress = currentThreadAddress;
	//int temporaryThreadAddress;
	OSThread *temporaryThreadAddress;

	// Follow "previous thread" pointers back to the beginning
	//while ((temporaryThreadAddress = (int *) (iterationThreadAddress + PREVIOUS_THREAD)) != 0)
	while ((temporaryThreadAddress = iterationThreadAddress->link.prev) != 0) {
		WHBLogPrintf("Temporary thread address going backwards: %08x\n", temporaryThreadAddress);
		iterationThreadAddress = temporaryThreadAddress;
	}

	// Now iterate over all threads
	//while ((temporaryThreadAddress = *(int *) (iterationThreadAddress + NEXT_THREAD)) != 0))
	while ((temporaryThreadAddress = iterationThreadAddress->link.next) != 0) {
		// Grab the thread's address
		WHBLogPrintf("Temporary thread address going forward: %08x\n", temporaryThreadAddress);
		threads = insert(threads, (void *) iterationThreadAddress);
		WHBLogPrintf("Inserted: %08x\n", iterationThreadAddress);
		iterationThreadAddress = temporaryThreadAddress;
	}

	// The previous while would skip the last thread so add it as well
	threads = insert(threads, (void *) iterationThreadAddress);
	WHBLogPrintf("Inserted: %08x\n", iterationThreadAddress);

	reverse(&threads);

	return threads;
}

