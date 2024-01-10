#include "command_io.h"
#include "../tcpgecko/assertions.h"

#include <nsysnet/socket.h>
#include <coreinit/debug.h>

//KNOWN ISSUE: Because of buffer originally being redefined per function,
//The current implementation sends a really big empty buffer instead of a 
//small buffer of just the relevant info

#define CHECK_ERROR(cond)     \
	if (cond)                 \
	{                         \
		line = __LINE__; \
		goto error;           \
	}

int CommandIO::recvwait_buffer(unsigned char *buffer, int len){
	while (len > 0) {
		ret = recv(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer += ret;
	}
	return 0;

	error:
	error = ret;
	return ret;
}

int CommandIO::recvwait(int len){
	while (len > 0) {
		ret = recv(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer += ret; //hmm
	}
	return 0;

	error:
	error = ret;
	return ret;
}

int CommandIO::recvbyte(){
	unsigned char buffer[1];

	ret = recvwait(1);
	if (ret < 0) return ret;
	return buffer[0];
}

int CommandIO::checkbyte(){
	unsigned char buffer[1];

	ret = recv(sock, buffer, 1, MSG_DONT_WAIT);
	if (ret < 0) return ret;
	if (ret == 0) return -1;
	return buffer[0];
}

int CommandIO::sendwait(int len){
    unsigned char *buffer;
	while (len > 0) {
		ret = send(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer += ret;
	}
	return;
	error:
	error = ret;
	return ret;
}

int CommandIO::sendByte(u_char byte){
	buffer[0] = byte;
	return sendwait(1);
}

unsigned int CommandIO::receiveString(unsigned char *stringBuffer, unsigned int bufferSize){
	// Receive the string length
	unsigned char lengthBuffer[4] = {0};
	int ret = recvwait_buffer(lengthBuffer, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (string length)");
	unsigned int stringLength = ((unsigned int *) lengthBuffer)[0];

	if (stringLength <= bufferSize) {
		// Receive the actual string
		ret = recvwait_buffer(stringBuffer, stringLength);
		ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (string)");
	} else {
		OSFatal("String buffer size exceeded");
	}

	return stringLength;
}
