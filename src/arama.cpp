//TCPGecko includes
#include "tcpgecko/title.h"
#include "utils/logger.h"
#include "tcpgecko/tcp_gecko.h"
#include <gctypes.h>
#include <stdlib.h> // malloc()/free()
#include <string.h> // memcpy()
#include <coreinit/filesystem.h>
#include <gx2/swap.h>
#include <gx2/surface.h>

// The dynamically allocated buffer size for the image copy
#define IMAGE_BUFFER_SIZE 100

// The time the producer and consumer wait while there is nothing to do
#define WAITING_TIME_MILLISECONDS 1

//aRAMa code
#include <wups.h>
#include <wups/storage.h>
#include <notifications/notifications.h>

//Metadata
WUPS_PLUGIN_NAME("aRAMa");
WUPS_PLUGIN_DESCRIPTION("RAM magic for Aroma");
WUPS_PLUGIN_VERSION("v0");
WUPS_PLUGIN_AUTHOR("Rewrite & Aroma port: c08o.prkiua \n Original legacy version: wj444 + contributors");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_WUT_MALLOC(); //Idrk
WUPS_USE_STORAGE("aRAMa");

INITIALIZE_PLUGIN(){
	
	/*WUPSStorageError storag_err = WUPS_OpenStorage();

	if (storag_err == WUPS_STORAGE_ERROR_SUCCESS){
		wups_storage_item_t sd_codes;
		sd_codes.key = ""
	}*/
		
	//look for the "use SD codes" setting

	//if that is true, check titleid

	//if the titleID has codes stored for it (also in storage), load them
	startTCPGecko();
}

DEINITIALIZE_PLUGIN(){

}

//Based on entry.c
ON_APPLICATION_START(){
	if (isRunningAllowedTitleID()){
		log_print("OSGetTitleID checks passed...\n");

	}
	//Todo: Figure out what all TCPGecko does on an application launch:

	//TCPGecko apparently would relaunch itself every time you open a title (maybe change that)

	//Apply game specific stored codes if that is enabled
}

ON_APPLICATION_REQUESTS_EXIT(){
	//Prolly some code that unpatches the game specific function patches TCPGecko employs
}

//Several hooks from the utils folder

//declareFunctionHook
DECL_FUNCTION(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling){
	log_printf("FSOpenFile(): %s", path);
	return real_FSOpenFile(pClient, pCmd, path, mode, fd, errHandling);
}

WUPS_MUST_REPLACE(FSOpenFile ,WUPS_LOADER_LIBRARY_COREINIT,FSOpenFile);

//declareFunctionHook
DECL_FUNCTION(int, FSOpenFileAsync, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error, const void *asyncParams) {
	log_printf("FSOpenFileAsync(): %s", path);
	return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle, error, asyncParams);
}

WUPS_MUST_REPLACE(FSOpenFileAsync,WUPS_LOADER_LIBRARY_COREINIT,FSOpenFileAsync);

//declareFunctionHook
DECL_FUNCTION(int, FSOpenDir, void *pClient, void *pCmd, const char *path, s32 *dh, int errHandling) {
	log_printf("FSOpenDir(): %s", path);
	return real_FSOpenDir(pClient, pCmd, path, dh, errHandling);
}

WUPS_MUST_REPLACE(FSOpenDir,WUPS_LOADER_LIBRARY_COREINIT,FSOpenDir);

//declareFunctionHook
DECL_FUNCTION(int, FSOpenDirAsync, void *pClient, void *pCmd, const char *path, int *handle, int error, void *asyncParams) {
	log_printf("FSOpenDirAsync(): %s", path);
	return real_FSOpenDirAsync(pClient, pCmd, path, handle, error, asyncParams);
}

WUPS_MUST_REPLACE(FSOpenDirAsync,WUPS_LOADER_LIBRARY_COREINIT,FSOpenDirAsync);

//declareFunctionHook
DECL_FUNCTION(int, FSGetStat, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling) {
	log_printf("FSGetStat(): %s", path);
	return real_FSGetStat(pClient, pCmd, path, stats, errHandling);
}

WUPS_MUST_REPLACE(FSGetStat,WUPS_LOADER_LIBRARY_COREINIT,FSGetStat);

//declareFunctionHook
DECL_FUNCTION(int, FSGetStatAsync, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling, void *asyncParams) {
	log_printf("FSGetStatAsync(): %s", path);
	return real_FSGetStatAsync(pClient, pCmd, path, stats, errHandling, asyncParams);
}

WUPS_MUST_REPLACE(FSGetStatAsync,WUPS_LOADER_LIBRARY_COREINIT,FSGetStatAsync);

static volatile int executionCounter = 0;

bool shouldTakeScreenShot = false;
unsigned int remainingImageSize = 0;
unsigned int totalImageSize = 0;
int bufferedImageSize = 0;
void *bufferedImageData = NULL;

//declareFunctionHook
DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, s32 scan_target){
	if (executionCounter > 120) {
		GX2Surface surface = colorBuffer->surface;
		log_printf("GX2CopyColorBufferToScanBuffer {surface width:%d, height:%d, image size:%d, image data:%x}\n",
				   surface.width, surface.height, surface.image, surface.image);

		if (shouldTakeScreenShot) {
			void *imageData = surface.image;
			totalImageSize = surface.image;
			remainingImageSize = totalImageSize;
			int bufferSize = IMAGE_BUFFER_SIZE;

			while (remainingImageSize > 0) {
				bufferedImageData = malloc(bufferSize);
				u32 imageSizeRead = totalImageSize - remainingImageSize;
				memcpy(bufferedImageData, imageData + imageSizeRead, bufferSize);
				bufferedImageSize = bufferSize;

				// Wait while the data is not read yet
				while (bufferedImageSize > 0) {
					usleep(WAITING_TIME_MILLISECONDS);
				}

				free(bufferedImageData);
				remainingImageSize -= bufferSize;
			}

			shouldTakeScreenShot = false;
		}
		/*s32 format = surface.format;

		gdImagePtr gdImagePtr = 0;
		bool no_convert;
		u8 *image_data = NULL;
		int img_size = 0;
		if (format == 0x1A) {
			UnormR8G8B8A82Yuv420p(&image_data, surface.image_data, &img_size, surface.width, surface.height,
								  surface.pitch);
		} else if (format == 0x19) {
			no_convert = true;
			UnormR8G8B8A8TogdImage(&gdImagePtr, surface.image_data, surface.width, surface.height, surface.pitch);
		} else {
			no_convert = true;
		}

		u32 imd_size = 0;
		void *data = gdImageJpegPtr(gdImagePtr, &imd_size, 95);
		if (data) {
			JpegData jpeg;
			jpeg.img_size = imd_size;
			jpeg.img_data = data;
			jpeg.img_id = 0;
		}*/

		executionCounter = 0;
	}

	executionCounter++;

	//real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
	return real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
}

WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
