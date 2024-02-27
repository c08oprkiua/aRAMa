#include <stdlib.h> // malloc()/free()
#include <string.h> // memcpy()

#include <wups.h>
#include <gctypes.h>

#include <gx2/swap.h>
#include <gx2/surface.h>

#include <notifications/notifications.h>

#include "../command_handler.h"

// The dynamically allocated buffer size for the image copy
#define IMAGE_BUFFER_SIZE 100

// The time the producer and consumer wait while there is nothing to do
#define WAITING_TIME_MILLISECONDS 1

#define WRITE_SCREEN_MESSAGE_BUFFER_SIZE 100

//Screenshot variables
static volatile int executionCounter = 0;
unsigned int remainingImageSize = 0;
unsigned int totalImageSize = 0;
int bufferedImageSize = 0;
void *bufferedImageData = nullptr;
bool shouldTakeScreenShot = false;

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, s32 scan_target){
	if (executionCounter > 120) {
		GX2Surface surface = colorBuffer->surface;
		WHBLogPrintf("GX2CopyColorBufferToScanBuffer {surface width:%d, height:%d, image size:%d, image data:%x}\n",
				   surface.width, surface.height, surface.imageSize, surface.image_data);

		if (shouldTakeScreenShot) {
			void *imageData = surface.image_data;
			totalImageSize = surface.imageSize;
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
		uint8_t *image_data = NULL;
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

		uint32_t imd_size = 0;
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

inline void CommandHandler::command_take_screenshot(){
	// Tell the hook to dump the screen shot now
	shouldTakeScreenShot = true;

	// Tell the client the size of the upcoming image
	ret = sendwait_buffer((unsigned char *)&totalImageSize, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (total image size)");

	// Keep sending the image data
	while (remainingImageSize > 0)
	{
		int bufferPosition = 0;

		// Fill the buffer till it is full
		while (bufferPosition <= DATA_BUFFER_SIZE)
		{
			// Wait for data to be available
			while (bufferedImageSize == 0)
			{
				usleep(WAITING_TIME_MILLISECONDS);
			}

			memcpy(buffer + bufferPosition, bufferedImageData, bufferedImageSize);
			bufferPosition += bufferedImageSize;
			bufferedImageSize = 0;
		}

		// Send the size of the current chunk
		ret = sendwait_buffer((unsigned char *)&bufferPosition, sizeof(int));
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image data chunk size)");

		// Send the image data itself
		ret = sendwait(bufferPosition);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image data)");
	}

	/*GX2ColorBuffer colorBuffer;
	// TODO Initialize colorBuffer!
	GX2Surface surface = colorBuffer.surface;
	void *image_data = surface.image_data;
	uint32_t image_size = surface.image_size;

	// Send the image size so that the client knows how much to read
	ret = sendwait(bss, clientfd, (unsigned char *) &image_size, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image size)")

	unsigned int imageBytesSent = 0;
	while (imageBytesSent < image_size) {
		int length = image_size - imageBytesSent;

		// Do not smash the buffer
		if (length > DATA_BUFFER_SIZE) {
			length = DATA_BUFFER_SIZE;
		}

		// Send the image bytes
		memcpy(buffer, image_data, length);
		ret = sendwait(bss, clientfd, buffer, length);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image bytes)")

		imageBytesSent += length;
	}*/
};

//TODO: Make this cause a notification to appear onscreen
inline void CommandHandler::command_write_screen(){
	char message[WRITE_SCREEN_MESSAGE_BUFFER_SIZE];
	ret = recvwait(4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (write screen seconds)")
	int seconds = ((int *)buffer)[0];
	receiveString(message, WRITE_SCREEN_MESSAGE_BUFFER_SIZE);
	writeScreen(message, seconds);

	break;
};