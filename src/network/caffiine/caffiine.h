#ifndef CAFFIINE_H
#define CAFFIINE_H

#include "./network/command_io/command_io.h"

class Caffiine: public CommandIO {
    void caf_connect(uint32_t ip);

    void disconnect();

    int handshake();

    int fsetpos(int *result, int fd, int set);

    int send_handle(const char *path, int handle);

    int fopen(int *result, const char *path, const char *mode, int *handle);

    void send_file(char *file, int size, int fd);

    int fread(int *result, void *ptr, int size, int fd);

    int fclose(int *result, int fd, int dumpclose);

    void send_ping(int val1, int val2);
};

#endif