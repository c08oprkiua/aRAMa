#ifndef SAVIINE_H
#define SAVIINE_H

#include "../command_io/command_io.h"

class Saviine: public CommandIO {
    int start_injection(long persistentID, int *mask);
    int end_injection();
    int start_dump(long persistentID, int *mask);
    int end_dump();
    int readdir(char *path, char *resultname, int *resulttype, int *filesize);
};

#endif