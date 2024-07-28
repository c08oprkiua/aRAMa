#ifndef CODE_HANDLER_H
#define CODE_HANDLER_H

#include "core/thread_server.h"

//This will likely be changed into an address pointer
#define CODE_HANDLER_INSTALL_ADDRESS 0x010F4000
#define ENDING_ADDRESS 0x10000000


class CodeHandler: public aRAMaServer {
private:
    //*mumble mumble* array of codes here

    uint32_t assemblySize = 0;

    uint32_t *install_address;
public:

    uint32_t code_count;

    void executeAssembly();

    void persistAssembly(uint32_t asm_size, uint8_t *source_buffer);

    uint32_t getStartAddress();

    void loadSDCodes();

    int run() override;

    void clear();
};


#endif