#include <stdio.h> // snprintf
#include <string.h> // memcpy, memset
//#include "../utils/logger.h"
#include <whb/log.h>
#include "../fs/sd_fat_devoptab.h"
#include "../fs/fs_utils.h"

//#include "../dynamic_libs/os_functions.h"
#include <coreinit/memorymap.h>
#include <coreinit/title.h>
#include <coreinit/cache.h>

#include "tcp_gecko.h"
//#include "../kernel/syscalls.h"
#include <kernel/kernel.h>

#include <fat.h>
#include <iosuhax.h>


#define CODE_HANDLER_ENABLED_ADDRESS 0x10014EFC
#define CODE_LIST_START_ADDRESS 0x01133000

#define EXTENSION_SIZE 6
#define SD_FILE_PATH_HEADER_LENGTH 10
#define TITLE_ID_LEADING_ZEROS 3
#define TITLE_ID_LENGTH 16
#define CODES_FILE_PATH_SIZE (SD_FILE_PATH_HEADER_LENGTH + TITLE_ID_LENGTH + EXTENSION_SIZE)

u64 cachedTitleID;

unsigned char *kernelCopyBufferOld2[DATA_BUFFER_SIZE];

//Literally identical to GeckoKernelCopyData but it uses the buffer in this file, lol
void kernelCopyData2(unsigned char *destinationBuffer, unsigned char *sourceBuffer, unsigned int length) {
	if (length > DATA_BUFFER_SIZE) {
		OSFatal("Kernel copy buffer size exceeded");
	}

	memcpy(kernelCopyBufferOld2, sourceBuffer, length);
	//SC0x25_KernelCopyData
	KernelCopyData(OSEffectiveToPhysical(destinationBuffer), (unsigned int) &kernelCopyBufferOld2, length);
	DCFlushRange(destinationBuffer, (u32) length);
}

void setCodeHandlerEnabled(bool enabled) {
	unsigned int *codeHandlerEnabled = (unsigned int *) CODE_HANDLER_ENABLED_ADDRESS;
	*codeHandlerEnabled = (unsigned int) enabled;
	WHBLogPrintf("Code handler status: %i\n", enabled);
}

/*void testMount() {
	int res = IOSUHAX_Open(NULL);
	WHBLogPrintf("Result: %i", res);

	if (res < 0) {//
		mount_sd_fat("sd"); // Fallback to normal OS implementation
	} else {
		fatInitDefault(); // using libfat
	}

	WHBLogPrint("Unmounting...");
	fatUnmount("sd");
	fatUnmount("usb");
	WHBLogPrint("Closing...");
	IOSUHAX_Close();
	WHBLogPrint("DONE");
}*/

void considerApplyingSDCheats() {
	u64 currentTitleID = OSGetTitleID();

	// testMount();

	if (cachedTitleID == currentTitleID) {
		// WHBLogPrint("Title ID NOT changed\n");
	} else {
		WHBLogPrint("Title ID changed\n");
		cachedTitleID = currentTitleID;
		WHBLogPrint("Mounting...\n");
		int result = mount_sd_fat("sd");

		if (result < 0) {
			WHBLogPrintf("Mounting error: %i\n", result);
			return;
		} else {
			WHBLogPrint("Mounted!\n");
		}

		// Construct the file path
		unsigned char filePath[CODES_FILE_PATH_SIZE];
		memset(filePath, '0', sizeof(filePath));
		memcpy(filePath, "sd:/codes/", SD_FILE_PATH_HEADER_LENGTH); // File path header
		char asciiTitleID[TITLE_ID_LENGTH];
		snprintf(asciiTitleID, TITLE_ID_LENGTH, "%llX", currentTitleID);
		WHBLogPrintf("Title ID: %s\n", asciiTitleID);
		memcpy(filePath + SD_FILE_PATH_HEADER_LENGTH + TITLE_ID_LEADING_ZEROS, asciiTitleID,
			   TITLE_ID_LENGTH); // Title ID
		memcpy(filePath + SD_FILE_PATH_HEADER_LENGTH + TITLE_ID_LENGTH, ".gctu", EXTENSION_SIZE); // Extension
		filePath[CODES_FILE_PATH_SIZE - 1] = '\0'; // Null-terminated
		WHBLogPrintf("File Path: %s\n", filePath);

		unsigned char *codes = NULL;
		uint32_t codesSize = 0;
		result = LoadFileToMem((const char *) filePath, &codes, &codesSize);

		if (result < 0) {
			WHBLogPrintf("LoadFileToMem() error: %i\n", result);
			setCodeHandlerEnabled(false);
			// Error, we won't write any codes
			goto CLEANUP;
		}

		WHBLogPrint("Copying...\n");
		kernelCopyData2((unsigned char *) CODE_LIST_START_ADDRESS, codes, (unsigned int) codesSize);
		WHBLogPrint("Copied!\n");
		setCodeHandlerEnabled(true);

		CLEANUP:

		WHBLogPrint("Unmounting...\n");
		result = unmount_sd_fat("sd");

		if (result < 0) {
			WHBLogPrintf("Unmounting error: %i\n", result);
		} else {
			WHBLogPrint("Unmouted!\n");
		}
	}
}