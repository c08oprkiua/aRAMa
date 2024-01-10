#pragma once

#include "../common/fs_defs.h"
#include "../utils/logger.h"
//#include "../dynamic_libs/os_functions.h"


#include <gd.h>

typedef struct
{
	uint32_t img_size;
	uint32_t img_id;
	void * img_data;
} JpegData;

typedef struct _R8G8B8A8_COLOR {
	uint8_t R, G, B, A;
} R8G8B8A8_COLOR;

void UnormR8G8B8A8TogdImage(gdImagePtr *gdImgTmp, void *image_data, uint32_t width, uint32_t rows, uint32_t pitch) {
	*gdImgTmp = gdImageCreateTrueColor(width / 2, rows / 2);

	R8G8B8A8_COLOR *buffer = (R8G8B8A8_COLOR *) image_data;
	R8G8B8A8_COLOR val;
	for (uint32_t row = 0; row < rows; ++row) {
		for (uint32_t x = 0; x < width; ++x) {
			val = buffer[row * pitch + x];
			gdImageSetPixel(*gdImgTmp, x / 2, row / 2, gdTrueColor(val.R, val.G, val.B));
			++x;
		}
		++row;
	}
}

void UnormR8G8B8A82Yuv420p(uint8_t **destination_, void *image_data, int *dest_img_size, uint32_t width, uint32_t height, uint32_t pitch) {
	uint32_t image_size = width * height;
	uint32_t upos = image_size;
	uint32_t vpos = upos + upos / 4;
	*dest_img_size = (vpos + upos / 4);
	if (*destination_) {
		free(destination_);
	}
	*destination_ = (uint8_t *) malloc(sizeof(u8) * *dest_img_size);
	uint8_t *destination = *destination_;
	if (!destination) {
		*dest_img_size = 0;
		return;
	}
	log_printf("allocated %d \n", *dest_img_size);

	R8G8B8A8_COLOR *buffer = (R8G8B8A8_COLOR *) image_data;

	uint32_t i = 0;

	for (uint32_t line = 0; line < height; ++line) {
		if (!(line % 2)) {
			for (uint32_t x = 0; x < width; x += 2) {
				uint8_t r = buffer[line * pitch + x].R;
				uint8_t g = buffer[line * pitch + x].G;
				uint8_t b = buffer[line * pitch + x].B;

				destination[i++] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;

				destination[upos++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
				destination[vpos++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;

				r = buffer[line * pitch + x].R;
				g = buffer[line * pitch + x].G;
				b = buffer[line * pitch + x].B;

				destination[i++] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
			}
		} else {
			for (uint32_t x = 0; x < width; x += 1) {
				uint8_t r = buffer[line * pitch + x].R;
				uint8_t g = buffer[line * pitch + x].G;
				uint8_t b = buffer[line * pitch + x].B;

				destination[i++] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
			}
		}
	}
	sleep(1);
	log_printf("done %d \n", *dest_img_size);
}

void UnormR10G10B10A2TogdImage(gdImagePtr *gdImgTmp, void *image_data, uint32_t width, uint32_t rows, uint32_t pitch) {
	uint32_t *buffer = (uint32_t *) image_data;
	uint32_t val = 0;
	for (uint32_t row = 0; row < rows; ++row) {
		for (uint32_t x = 0; x < width; ++x) {
			/*
				R ((test >> 24) & 0xFF))
				G ((test >> 14) & 0xFF))
				B ((test >> 4) & 0xFF))
				alpha (test & 0x3);
			*/
			val = buffer[row * pitch + x];
			gdImageSetPixel(*gdImgTmp, x, row, gdTrueColor(((val >> 24) & 0xFF),
														   ((val >> 14) & 0xFF),
														   ((val >> 4) & 0xFF)));
		}
	}
}