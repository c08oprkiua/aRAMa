#include <stdlib.h> // malloc()/free()
#include <string.h> // memcpy()

#include <wups.h>
#include <gctypes.h>

#include <gx2/swap.h>
#include <gx2/surface.h>

#include "arama.h"

// The dynamically allocated buffer size for the image copy
#define IMAGE_BUFFER_SIZE 100

// The time the producer and consumer wait while there is nothing to do
#define WAITING_TIME_MILLISECONDS 1

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, s32 scan_target){
	if (executionCounter > 120) {
		GX2Surface surface = colorBuffer->surface;
		log_printf("GX2CopyColorBufferToScanBuffer {surface width:%d, height:%d, image size:%d, image data:%x}\n",
				   surface.width, surface.height, surface.image_size, surface.image_data);

		if (shouldTakeScreenShot) {
			void *imageData = surface.image_data;
			totalImageSize = surface.image_size;
			remainingImageSize = totalImageSize;
			int bufferSize = IMAGE_BUFFER_SIZE;

			while (remainingImageSize > 0) {
				bufferedImageData = malloc(bufferSize);
				uint32_t imageSizeRead = totalImageSize - remainingImageSize;
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