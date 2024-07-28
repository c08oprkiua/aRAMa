#include "../command_handler.h"
#include "core/node_list_plus_plus.h"
#include <stdio.h>
#include <stdlib.h>

NodeList<OSThread> *getAllThreads(NodeList<OSThread> *threads){
	OSThread *currentThreadAddress = OSGetCurrentThread();
	//log_printf("Thread address: %08x\n", currentThreadAddress);
	OSThread *iterationThreadAddress = currentThreadAddress;
	OSThread *temporaryThreadAddress;

	// Follow "previous thread" pointers back to the beginning
	while ((temporaryThreadAddress = iterationThreadAddress->activeLink.prev) != nullptr) {
		//log_printf("Temporary thread address going backwards: %08x\n", temporaryThreadAddress);
		iterationThreadAddress = temporaryThreadAddress;
	}

	// Now iterate over all threads
	while ((temporaryThreadAddress = iterationThreadAddress->activeLink.next) != nullptr) {
		// Grab the thread's address
		//log_printf("Temporary thread address going forward: %08x\n", temporaryThreadAddress);
		threads->insert(*iterationThreadAddress);
		//log_printf("Inserted: %08x\n", iterationThreadAddress);
		iterationThreadAddress = temporaryThreadAddress;
	}

	// The previous while would skip the last thread so add it as well
	threads->insert(*iterationThreadAddress);
	//log_printf("Inserted: %08x\n", iterationThreadAddress);

	threads->reverse();
}


void CommandHandler::command_read_threads(){
	NodeList<OSThread> *new_threads;
	new_threads = getAllThreads(new_threads);
	int threadCount = new_threads->length();
	WHBLogPrintf("Thread Count: %i\n", threadCount);

	// Send the thread count
	WHBLogPrint("Sending thread count...\n");
	((int *)buffer)[0] = threadCount;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (thread count)");

	// Send the thread addresses and data
	Node<OSThread> *currentThread = new_threads->get_list();
	while (currentThread != NULL){
		int data = (int)currentThread->data;
		WHBLogPrintf("Thread data: %08x\n", data);
		((int *)buffer)[0] = (int)currentThread->data;
		memcpy(buffer + sizeof(int), currentThread->data, sizeof(OSThread));
		WHBLogPrint("Sending Node...\n");
		ret = sendwait(sizeof(int) + sizeof(OSThread));
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (thread address and data)")

		currentThread = currentThread->next;
	}

};