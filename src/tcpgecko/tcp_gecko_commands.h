//I went back to the macros. Also, see gecko_commands.h

//from os_functions.h
#define PPC_DISASM_MAX_BUFFER 64

#define PPC_DISASM_DEFAULT     0  // use defaults
#define PPC_DISASM_SIMPLIFY    0x00000001  // use simplified mnemonics
#define PPC_DISASM_REG_SPACES  0x00000020  // emit spaces between registers
#define PPC_DISASM_EMIT_DISASM 0x00000040  // emit only disassembly
#define PPC_DISASM_EMIT_ADDR   0x00000080  // emit only addresses + disassembly
#define PPC_DISASM_EMIT_FUNCS  0x00000100  // emit function names before and during disassembly