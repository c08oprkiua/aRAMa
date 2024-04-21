#include "caffiine.h"

#include <nsysnet/socket.h>
#include <coreinit/title.h>

void Caffiine::caf_connect(uint32_t server_ip) {
	struct sockaddr_in addr;

    int *psock;

	//socket_lib_init(); //wdym

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (check_error(sock == -1, FAIL_SOCKET_INIT, 5)){
		goto error;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = 7332;
	addr.sin_addr.s_addr = server_ip;

	ret = connect(sock, (sockaddr *) &addr, sizeof(addr));
	if (check_error(ret < 0, FAIL_SOCKET_CONNECT, 1)){
		goto error;
	}

	ret = handshake();
	if (check_error(ret < 0, FAIL_SOCKET_HANDSHAKE, 1)){
		goto error;
	}
	CHECK_ERROR(ret < 0);
	CHECK_ERROR(ret == BYTE_NORMAL);

	*psock = sock;
	return;
	
	error:
	if (sock != -1)
		socketclose(sock);
	*psock = -1;
	return;
}

void Caffiine::disconnect() {
	CHECK_ERROR(sock == -1);
	socketclose(sock);
	error:
	return;
}

int Caffiine::handshake() {
	int ret;

	unsigned char buffer[16];

	uint64_t title_id = OSGetTitleID();
	memcpy(buffer, &title_id, 16);

	ret = sendwait(sizeof(buffer));
	CHECK_ERROR(ret < 0);
	ret = recvbyte();
	CHECK_ERROR(ret < 0);
	return ret;
	error:
	return ret;
}

int Caffiine::fsetpos(int *result, int fd, int set) {
	while (iLock)
		usleep(5000);
	iLock = 1;

	CHECK_ERROR(sock == -1);

	int ret;
	char buffer[1 + 8];
	buffer[0] = BYTE_SETPOS;
	*(int *) (buffer + 1) = fd;
	*(int *) (buffer + 5) = set;
	ret = sendwait(1 + 8);
	CHECK_ERROR(ret < 0);
	ret = recvbyte();
	CHECK_ERROR(ret < 0);
	CHECK_ERROR(ret == BYTE_NORMAL);
	ret = recvwait_buffer( (unsigned char *) result, 4);
	CHECK_ERROR(ret < 0);

	iLock = 0;
	return 0;
	error:
	iLock = 0;
	return -1;
}

int Caffiine::send_handle(const char *path, int handle) {
	while (iLock)
		usleep(5000);
	iLock = 1;

	CHECK_ERROR(sock == -1);

	// create and send buffer with : [cmd id][handle][path length][path data ...]
	{
		int ret;
		int len_path = 0;
		while (path[len_path++]);
		char buffer[1 + 4 + 4 + len_path];

		buffer[0] = BYTE_HANDLE;
		*(int *) (buffer + 1) = handle;
		*(int *) (buffer + 5) = len_path;
		for (ret = 0; ret < len_path; ret++)
			buffer[9 + ret] = path[ret];

		// send buffer, wait for reply
		ret = sendwait(1 + 4 + 4 + len_path);
		CHECK_ERROR(ret < 0);

		// wait reply
		ret = recvbyte();
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret == BYTE_SPECIAL);
		if (ret == BYTE_REQUEST) {
			ret = 1;
		} else {
			ret = 2;
		}
		// wait reply
		int special_ret = recvbyte();
		CHECK_ERROR(special_ret < 0);
		CHECK_ERROR(special_ret != BYTE_SPECIAL);
		iLock = 0;
		return ret;
	}

	error:
	iLock = 0;
	return -1;
}

/*int Caffiine::fopen(int sock, int *result, const char *path, const char *mode, int *handle) {
	while (iLock)
		usleep(5000);
	iLock = 1;

	int final_result = -1;
	CHECK_ERROR(sock == -1);

	int ret;
	int len_path = 0;
	len_path++;
	// while (path[len_path++]);
	int len_mode = 0;
	len_mode++;
	// while (mode[len_mode++]);

	//
	{
		char buffer[1 + 8 + len_path + len_mode];
		buffer[0] = BYTE_OPEN;
		*(int *) (buffer + 1) = len_path;
		*(int *) (buffer + 5) = len_mode;
		for (ret = 0; ret < len_path; ret++)
			buffer[9 + ret] = path[ret];
		for (ret = 0; ret < len_mode; ret++)
			buffer[9 + len_path + ret] = mode[ret];

		ret = sendwait(sock, buffer, 1 + 8 + len_path + len_mode);
		CHECK_ERROR(ret < 0);

		ret = recvbyte(sock);
		CHECK_ERROR(ret < 0);
		CHECK_ERROR(ret == BYTE_NORMAL);

		ret = recvwait(sock, result, 4);
		CHECK_ERROR(ret < 0);
		ret = recvwait(sock, handle, 4);
		CHECK_ERROR(ret < 0);
	}
	final_result = 0;


	error:
	iLock = 0;
	return final_result;
}*/

void Caffiine::send_file(char *file, int size, int fd) {
	while (iLock)
		usleep(5000);
	iLock = 1;

	CHECK_ERROR(sock == -1);

	int ret;

	// create and send buffer with : [cmd id][fd][size][buffer data ...]
	{
		char buffer[1 + 4 + 4 + size];

		buffer[0] = BYTE_DUMP;
		*(int *) (buffer + 1) = fd;
		*(int *) (buffer + 5) = size;
		for (ret = 0; ret < size; ret++)
			buffer[9 + ret] = file[ret];

		// send buffer, wait for reply
		ret = sendwait(1 + 4 + 4 + size);
		CHECK_ERROR(ret < 0);
		// wait reply
		ret = recvbyte();
		CHECK_ERROR(ret != BYTE_SPECIAL);
	}

	error:
	iLock = 0;
	return;
}

int Caffiine::fread(int *result, void *ptr, int size, int fd) {
	while (iLock)
		usleep(5000);
	iLock = 1;

	CHECK_ERROR(sock == -1);

	char buffer[1 + 8];
	buffer[0] = BYTE_READ;
	*(int *) (buffer + 1) = size;
	*(int *) (buffer + 5) = fd;
	ret = sendwait(1 + 8);
	CHECK_ERROR(ret < 0);
	ret = recvbyte();
	CHECK_ERROR(ret == BYTE_NORMAL);
	int sz;
	ret = recvwait_buffer((unsigned char *) &sz, 4);
	CHECK_ERROR(ret < 0);
	ret = recvwaitlen_buffer(ptr, sz);
	*result = sz - ret;
	ret = sendByte(BYTE_OK);
	CHECK_ERROR(ret < 0);

	iLock = 0;
	return 0;
	error:
	iLock = 0;
	return -1;
}

int Caffiine::fclose( int *result, int fd, int dumpclose) {
	while (iLock)
		usleep(5000);
	iLock = 1;

	CHECK_ERROR(sock == -1);

	char buffer[1 + 4];
	buffer[0] = BYTE_CLOSE;
	if (dumpclose)buffer[0] = BYTE_CLOSE_DUMP;
	*(int *) (buffer + 1) = fd;
	ret = sendwait(1 + 4);
	CHECK_ERROR(ret < 0);
	ret = recvbyte();
	CHECK_ERROR(ret == BYTE_NORMAL);
	ret = recvwait_buffer((unsigned char *) result, 4);
	CHECK_ERROR(ret < 0);

	iLock = 0;
	return 0;
	error:
	iLock = 0;
	return -1;
}
