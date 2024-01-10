#ifndef TCPGECKO_STACK_H
#define TCPGECKO_STACK_H

#include "linked_list.h"
//#include "cafe.h"
#include <coreinit/context.h>

bool isValidStackPointer(uint32_t stackPointer) {
	return stackPointer >= 0x10000000 && stackPointer < 0x20000000;
}

struct node *getStackTrace(OSContext *context) {
	struct node *stackTrace = NULL;
	uint32_t stackPointer = context->gpr[1];
	uint32_t stackPointersCount = 0;

	while (isValidStackPointer(stackPointer)) {
		stackPointer = *(uint32_t *) stackPointer;
		if (!isValidStackPointer(stackPointer)) {
			break;
		}

		int data = *(uint32_t * )(stackPointer + 4);
		stackTrace = insert(stackTrace, (void *) data);
		stackPointersCount++;
	}

	return stackTrace;
}

#endif
