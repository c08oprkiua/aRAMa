#ifndef NET_COMMON_H
#define NET_COMMON_H

/*
#include "command_io/command_io.h"
#include "caffiine/caffiine.h"
#include "saviine/saviine.h"
*/

#define BYTE_OPEN             0x00
#define BYTE_READ             0x01
#define BYTE_CLOSE            0x02
#define BYTE_OK               0x03
#define BYTE_SETPOS           0x04
#define BYTE_STATFILE         0x05
#define BYTE_EOF              0x06
#define BYTE_GETPOS           0x07
#define BYTE_REQUEST          0x08
#define BYTE_REQUEST_SLOW     0x09
#define BYTE_HANDLE        	  0x0A
#define BYTE_DUMP          	  0x0B
#define BYTE_PING          	  0x0C
#define BYTE_G_MODE           0x0D
#define BYTE_MODE_D           0x0E
#define BYTE_MODE_I           0x0F
#define BYTE_CLOSE_DUMP       0x10

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