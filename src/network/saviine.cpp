#include "saviine.h"

#include <string.h>
#include <malloc.h>
#include <unistd.h>

int Saviine::start_injection(long persistentID, int *mask) {
	while (fsMutex)
		usleep(5000);
	fsMutex = 1;

	int result = 0;
	CHECK_ERROR(sock == -1);
	{
		char buffer[1 + 4];

		buffer[0] = BYTE_INJECTSTART;
		*(long *) (buffer + 1) = persistentID;
		ret = sendwait(1 + 4);
		CHECK_ERROR(ret < 0);

		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret != BYTE_SPECIAL);

		ret = recvwait_buffer((uint8_t *) mask, 4);
		CHECK_ERROR(ret < 0);
		CHECK_ERROR((*mask & MASK_NORMAL) != MASK_NORMAL);

		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret != BYTE_SPECIAL);
		result = 1;
	}
	error:
	fsMutex = 0;
	return result;
}

int Saviine::end_injection() {
	while (fsMutex)
		usleep(5000);
	fsMutex = 1;

	int result = 0;
	CHECK_ERROR(sock == -1);
	{
		ret = sendByte(BYTE_INJECTEND);
		CHECK_ERROR(ret < 0);

		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret != BYTE_OK);
		result = 1;
	}
	error:
	fsMutex = 0;
	return result;
}

int Saviine::start_dump(long persistentID, int *mask) {
	while (fsMutex)
		usleep(5000);
	fsMutex = 1;

	int result = 0;
	CHECK_ERROR(sock == -1);
	{
		char buffer[1 + 4];

		buffer[0] = BYTE_DUMPSTART;
		*(long *) (buffer + 1) = persistentID;
		ret = sendwait(1 + 4);
		CHECK_ERROR(ret < 0);

		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret != BYTE_SPECIAL);

		ret = recvwait(4);
		CHECK_ERROR(ret < 0);
		CHECK_ERROR((*mask & MASK_NORMAL) != MASK_NORMAL);

		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret != BYTE_SPECIAL);
		result = 1;
	}
	error:
	fsMutex = 0;
	return result;
}

int Saviine::end_dump() {
	while (fsMutex)
		usleep(5000);
	fsMutex = 1;

	int result = 0;
	CHECK_ERROR(sock == -1);
	{
		ret = sendByte(BYTE_DUMPEND);
		CHECK_ERROR(ret < 0);

		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret != BYTE_OK);
		result = 1;
	}
	error:
	fsMutex = 0;
	return result;
}

int Saviine::readdir(char *path, char *resultname, int *resulttype, int *filesize) {
	while (fsMutex)
		usleep(5000);
	fsMutex = 1;

	int result = 0;
	CHECK_ERROR(sock == -1);
	// create and send buffer with : [cmd id][len_path][path][filesize]
	{
		int size = 0;
		while (path[size++]);
		char buffer[1 + 4 + size];

		buffer[0] = BYTE_READ_DIR;
		*(int *) (buffer + 1) = size;
		for (ret = 0; ret < size; ret++)
			buffer[5 + ret] = path[ret];

		// send buffer, wait for reply
		ret = sendwait(1 + 4 + size);
		CHECK_ERROR(ret < 0);

		// wait reply
		ret = recvbyte();
		CHECK_ERROR(ret != BYTE_OK);

		ret = recvbyte();
		CHECK_ERROR(ret != BYTE_FILE && ret != BYTE_FOLDER);
		*resulttype = ret;
		size = 0;
		ret = recvwait_buffer((uint8_t *) &size, 4);
		CHECK_ERROR(ret < 0);

		ret = recvwait_buffer((uint8_t *) resultname, size + 1);
		CHECK_ERROR(ret < 0);

		size = 0;
		ret = recvwait_buffer(&size, 4);
		CHECK_ERROR(ret < 0);
		*filesize = size;
		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret != BYTE_SPECIAL);
		result = 1;

	}
	error:
	fsMutex = 0;
	return result;
}