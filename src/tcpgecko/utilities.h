#ifndef TCPGECKO_UTILITIES_H
#define TCPGECKO_UTILITIES_H

//#include "../dynamic_libs/os_functions.h"
#include <coreinit/cache.h>

//#include "../utils/logger.h"
#include <whb/log.h>
#include "kernel.h"
#include <stddef.h> /* size_t */

void writeCode(uint32_t address, uint32_t instruction) {
	uint32_t *pointer = (uint32_t *) (address + 0xA0000000);
	*pointer = instruction;
	DCFlushRange(pointer, 4);
	ICInvalidateRange(pointer, 4);
}

/* Neither of these are used...?

void patchFunction(char *function, char *patchBytes, unsigned int patchBytesSize, int functionOffset) {
	WHBLogPrint("Patching function...\n");
	void *patchAddress = function + functionOffset;
	WHBLogPrintf("Patch address: %p\n", patchAddress);
	kernelCopyInt((unsigned char *) patchAddress, (unsigned char *) patchBytes, patchBytesSize);
	WHBLogPrint("Successfully patched!\n");
}

int memoryCompare(const void *s1, const void *s2, size_t n) {
	const unsigned char *p1 = static_cast<const unsigned char *>(s1);
	const unsigned char *p2 = static_cast<const unsigned char *>(s2);
	while (n--)
		if (*p1 != *p2)
			return *p1 - *p2;
		else
			p1++, p2++;
	return 0;
}
*/
#endif