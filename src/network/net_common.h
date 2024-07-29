#ifndef NET_COMMON_H
#define NET_COMMON_H

enum ByteCodes {
    BYTE_OPEN,
    BYTE_READ,
    BYTE_CLOSE,
    BYTE_OK,
    BYTE_SETPOS,
    BYTE_STATFILE,
    BYTE_EOF,
    BYTE_GETPOS,
    BYTE_REQUEST,
    BYTE_REQUEST_SLOW,
    BYTE_HANDLE,
    BYTE_DUMP,
    BYTE_PING,
    BYTE_G_MODE,
    BYTE_MODE_D,
    BYTE_MODE_I,
    BYTE_CLOSE_DUMP,

};

#define BYTE_FILE          	  0xC0
#define BYTE_FOLDER           0xC1

#define BYTE_READ_DIR         0xCC

#define BYTE_INJECTSTART      0x40
#define BYTE_INJECTEND        0x41
#define BYTE_DUMPSTART        0x42
#define BYTE_DUMPEND          0x43

#define BYTE_LOG_STR          0xfb

#define BYTE_END              0xfd
#define BYTE_SPECIAL          0xfe
#define BYTE_NORMAL           0xff

#define MASK_NORMAL           0x8000
#define MASK_USER             0x0100
#define MASK_COMMON           0x0200
#define MASK_COMMON_CLEAN     0x0400


#endif