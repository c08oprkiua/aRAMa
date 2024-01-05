#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
//#include "../dynamic_libs/os_functions.h"
#include <coreinit/screen.h>

//#include "../dynamic_libs/fs_functions.h"


//#include "../dynamic_libs/sys_functions.h"


//#include "../dynamic_libs/vpad_functions.h"
#include <vpad/input.h>

//#include "../dynamic_libs/socket_functions.h"


#include "../kernel/kernel_functions.h"
#include "../system/memory.h"
#include "../common/common.h"
#include "main.h"
#include "code_handler.h"
#include "../utils/logger.h"
#include "../utils/function_patcher.h"
#include "../patcher/function_patcher_gx2.h"
#include "../patcher/function_patcher_coreinit.h"
#include "sd_ip_reader.h"
#include "title.h"
#include "tcp_gecko.h"

bool isCodeHandlerInstalled = false;
bool areSDCheatsEnabled = false;

typedef enum {
	EXIT,
	TCP_GECKO
} LaunchMethod;

void applyFunctionPatches() {
	patchIndividualMethodHooks(method_hooks_gx2, method_hooks_size_gx2, method_calls_gx2);
	patchIndividualMethodHooks(method_hooks_coreinit, method_hooks_size_coreinit, method_calls_coreinit);
}

void installCodeHandler() {
	unsigned int physicalCodeHandlerAddress = (unsigned int) OSEffectiveToPhysical(
			(void *) CODE_HANDLER_INSTALL_ADDRESS);
	SC0x25_KernelCopyData((u32) physicalCodeHandlerAddress, (unsigned int) codeHandler, codeHandlerLength);
	DCFlushRange((const void *) CODE_HANDLER_INSTALL_ADDRESS, (u32) codeHandlerLength);
	isCodeHandlerInstalled = true;
}

unsigned char *screenBuffer;

#define PRINT_TEXT(x, y, ...) { snprintf(messageBuffer, 80, __VA_ARGS__); OSScreenPutFontEx(0, x, y, messageBuffer); OSScreenPutFontEx(1, x, y, messageBuffer); }

void initializeScreen() {
	// Init screen and screen buffers
	OSScreenInit();
	unsigned int screenBuffer0Size = OSScreenGetBufferSizeEx(SCREEN_TV);
	unsigned int screenBuffer1Size = OSScreenGetBufferSizeEx(SCREEN_DRC);

	screenBuffer = (unsigned char *) MEM1_alloc(screenBuffer0Size + screenBuffer1Size, 0x40);

	OSScreenSetBufferEx(0, screenBuffer);
	OSScreenSetBufferEx(1, (screenBuffer + screenBuffer0Size));

	OSScreenEnableEx(0, 1);
	OSScreenEnableEx(1, 1);
}

void install() {
	installCodeHandler();
	initializeUDPLog();
	log_print("Patching functions\n");
	applyFunctionPatches();
}

/* Entry point */
int Menu_Main(void) {
	//!*******************************************************************
	//!                   Initialize function pointers                   *
	//!*******************************************************************
	//! do OS (for acquire) and sockets first so we got logging
	InitOSFunctionPointers();
	InitSocketFunctionPointers();
	InitFSFunctionPointers();
	InitVPadFunctionPointers();
	InitSysFunctionPointers();

	if (strcasecmp("men.rpx", cosAppXmlInfoStruct.rpx_name) == 0) {
		return EXIT_RELAUNCH_ON_LOAD;
	} else if (strlen(cosAppXmlInfoStruct.rpx_name) > 0 &&
			   strcasecmp("ffl_app.rpx", cosAppXmlInfoStruct.rpx_name) != 0) {

		return EXIT_RELAUNCH_ON_LOAD;
	}

	//! *******************************************************************
	//! *                     Setup EABI registers                        *
	//! *******************************************************************
	register int old_sdata_start, old_sdata2_start;
	asm volatile (
	"mr %0, 13\n"
			"mr %1, 2\n"
			"lis 2, __sdata2_start@h\n"
			"ori 2, 2,__sdata2_start@l\n" // Set the Small Data 2 (Read Only) base register.
			"lis 13, __sdata_start@h\n"
			"ori 13, 13, __sdata_start@l\n"// # Set the Small Data (Read\Write) base register.
	: "=r" (old_sdata_start), "=r" (old_sdata2_start)
	);

	//!*******************************************************************
	//!                    Initialize BSS sections                       *
	//!*******************************************************************
	asm volatile (
	"lis 3, __bss_start@h\n"
			"ori 3, 3,__bss_start@l\n"
			"lis 5, __bss_end@h\n"
			"ori 5, 5, __bss_end@l\n"
			"subf 5, 3, 5\n"
			"li 4, 0\n"
			"bl memset\n"
	);

	SetupKernelCallback();
	// PatchMethodHooks();

	memoryInitialize();
	VPADInit();
	initializeScreen();

	char messageBuffer[80];
	int launchMethod;
	int shouldUpdateScreen = 1;
	VPADReadError vpadError = -1;
	VPADStatus vpad_data;

	while (true) {
		VPADRead(VPAD_CHAN_0, &vpad_data, 1, &vpadError);

		if (shouldUpdateScreen) {
			OSScreenClearBufferEx(0, 0);
			OSScreenClearBufferEx(1, 0);

			InitSocketFunctionPointers();

			// Build the IP address message
			char ipAddressMessageBuffer[64];
			__os_snprintf(ipAddressMessageBuffer, 64, "Your Wii U's IP address: %i.%i.%i.%i",
						  (hostIpAddress >> 24) & 0xFF, (hostIpAddress >> 16) & 0xFF, (hostIpAddress >> 8) & 0xFF,
						  hostIpAddress & 0xFF);

			PRINT_TEXT(14, 1, "-- TCP Gecko Installer --")
			PRINT_TEXT(7, 2, ipAddressMessageBuffer)
			PRINT_TEXT(0, 5, "Press A to install TCP Gecko (with built-in code handler)...")
			PRINT_TEXT(0, 6, "Press X to install TCP Gecko (with code handler and SD cheats)...")

			PRINT_TEXT(0, 8, "Note:")
			PRINT_TEXT(0, 9, "* You can enable loading SD cheats with Mocha SD access")
			PRINT_TEXT(0, 10, "* Generate and store GCTUs to your SD card with JGecko U")

			// testMount();
			/*if (isSDAccessEnabled()) {
				PRINT_TEXT2(0, 8, "SD card access: SD cheats will be applied automatically when titles are loaded!")
			} else {
				PRINT_TEXT2(0, 8, "No SD card access: Please run Mocha SD Access by maschell for SD cheat support...")
			}*/

			PRINT_TEXT(0, 17, "Press Home to exit...")

			OSScreenFlipBuffersEx(0);
			OSScreenFlipBuffersEx(1);
		}

		u32 pressedButtons = vpad_data.trigger | vpad_data.hold;

		// Home Button
		if (pressedButtons & VPAD_BUTTON_HOME) {
			launchMethod = EXIT;

			break;
		} else if (pressedButtons & VPAD_BUTTON_A) {
			install();
			launchMethod = TCP_GECKO;

			break;
		} else if (pressedButtons & VPAD_BUTTON_X) {
			install();
			launchMethod = TCP_GECKO;
			areSDCheatsEnabled = true;

			break;
		}

		// Button pressed?
		shouldUpdateScreen = (pressedButtons &
							  (VPAD_BUTTON_LEFT | VPAD_BUTTON_RIGHT | VPAD_BUTTON_UP | VPAD_BUTTON_DOWN)) ? 1 : 0;
		usleep(20 * 1000);
	}

	asm volatile ("mr 13, %0" : : "r" (old_sdata_start));
	asm volatile ("mr 2,  %0" : : "r" (old_sdata2_start));

	MEM1_free(screenBuffer);

	memoryRelease();

	if (launchMethod == EXIT) {
		// Exit the installer
		return EXIT_SUCCESS;
	} else {
		// Launch system menu
		SYSLaunchMenu();
	}

	// For each title load, relaunch the TCP Gecko
	return EXIT_RELAUNCH_ON_LOAD;
}

//Aroma WUPS code

#include <wups.h>

WUPS_PLUGIN_NAME("aRAMa")
WUPS_PLUGIN_DESCRIPTION("RAM magic for Aroma")
WUPS_PLUGIN_AUTHOR("port: c08o.prkiua, original: BullyWiiPlaza + contributors")

ON_APPLICATION_START(){
	//Todo: Figure out what all TCPGecko does on an application launch
}