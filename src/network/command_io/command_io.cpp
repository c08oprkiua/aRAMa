#include "command_io.h"
#include "./tcpgecko/assertions.h"

#include <nsysnet/socket.h>
#include <coreinit/debug.h>

int CommandIO::getMode(int * result){
	while (iLock)
		usleep(5000);
	iLock = 1;
	CHECK_ERROR(sock == -1);

	// create and send buffer with : [cmd id][fd][size][buffer data ...]
	{
		ret = sendByte(BYTE_G_MODE);

		// wait reply
		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		if (ret == BYTE_MODE_D) *result = BYTE_MODE_D;
		if (ret == BYTE_MODE_I) *result = BYTE_MODE_I;
		ret = 1;
	}
	error:
	iLock = 0;
	return ret;
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
	ret = recvwait(1);
	if (ret < 0) return ret;
	return buffer[0];
}

int CommandIO::checkbyte(){
	ret = recv(sock, buffer, 1, MSG_DONT_WAIT);
	if (ret < 0) return ret;
	if (ret == 0) return -1;
	return buffer[0];
}

int CommandIO::sendwait(int len){
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

int CommandIO::recvwaitlen(int len) {
	while (len > 0) {
		ret = recv(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer = &buffer + ret;
	}
	return 0;

	error:
	return len;
}

int CommandIO::recvwaitlen_buffer(void * buffer, int len) {
	while (len > 0) {
		ret = recv(sock, buffer, len, 0);
		CHECK_ERROR(ret < 0);
		len -= ret;
		buffer = &buffer + ret;
	}
	return 0;

	error:
	return len;
}

void CommandIO::log_string(const char *str, char flag_byte){
	if (sock == -1) {
		return;
	}
	while (iLock)
		usleep(5000);
	iLock = 1;

	int i;
	int len_str = 0;
	len_str++;
	// while (str[len_str++]);

	//
	{
		char buffer[1 + 4 + len_str];
		buffer[0] = flag_byte;
		*(int *) (buffer + 1) = len_str;
		for (i = 0; i < len_str; i++)
			buffer[5 + i] = str[i];

		buffer[5 + i] = 0;

		sendwait(1 + 4 + len_str);
	}

	iLock = 0;
}