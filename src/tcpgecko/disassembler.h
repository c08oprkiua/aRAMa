#ifndef TCPGECKO_DISASSEMBLER_H
#define TCPGECKO_DISASSEMBLER_H

#include <coreinit/debug.h>

//void formatDisassembled(char *format, ...);
DisassemblyPrintFn formatDisassembled(const char *format, ...);

#endif