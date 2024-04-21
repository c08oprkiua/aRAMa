#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "../network/command_io.h"

#include <whb/log.h>

class CommandHandler: public CommandIO
{
public:


    static void reportIllegalCommandByte(int byte);

    /*
    All of these are to preserve the original function's structure, 
    while also debloating that function for easier maintenence.
    */
    //comm_write_int
    void command_write_8();
    void command_write_16();
    void command_write_32();

    //comm_memory
    void command_read_memory();
    void command_read_memory_kernel();
    void command_memory_search_32();
    void command_advanced_memory_search();
    void command_read_memory_compressed();
    void command_upload_memory();

    //command_handler
        //comm_memory?
    void command_validate_address_range();
        //comm_debug?
    void command_read_threads();
    void command_follow_pointer();
    void command_remote_procedure_call();
    void command_get_symbol();
    void command_poke_registers();
    void command_get_stack_trace();
    void commmand_get_entry_point_address();

    //comm_disassemble
    void command_disassemble_range();
    void command_memory_disassemble();

    //comm_kernel
    void command_kernel_write();
    void command_kernel_read();
    void command_run_kernel_copy_service();

    //comm_screen
    void command_take_screenshot();
    void command_write_screen();


    //comm_metainfo
    void command_server_status();
    void command_server_version();
    void command_get_data_buffer_size();
    void command_get_code_handler_address();
    void command_account_identifier();
    void command_get_os_version();
    void command_get_version_hash();


    //comm_fs
    void command_read_file();
    void command_read_directory();
    void command_replace_file();
    void command_iosu_hax_read_file();

    //comm_asm
    void command_execute_assembly();
    void command_persist_assembly();
    void command_clear_assembly();

    //comm_pause
    void command_pause_console();
    void command_resume_console();
    void command_is_console_paused();

    //comm_breakpoint
    void command_set_data_breakpoint();
    void command_set_instruction_breakpoint();
    void command_toggle_breakpoint();
    void command_remove_all_breakpoints();

};

#endif