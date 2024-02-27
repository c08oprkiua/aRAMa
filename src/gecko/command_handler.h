#include "../network/command_io/command_io.h"

#include <whb/log.h>

class CommandHandler: public CommandIO
{
public:


    static void reportIllegalCommandByte(int byte);

    /*
    All of these are inline to preserve the original function's structure, 
    while also debloating that function for easier maintenence.
    */
    inline void command_write_8();
    inline void command_write_16();
    inline void command_write_32();

    inline void command_read_memory();
    inline void command_read_memory_kernel();
    inline void command_memory_search_32();
    inline void command_advanced_memory_search();

    inline void command_validate_address_range();
    inline void command_disassemble_range();
    inline void command_memory_disassemble();
    inline void command_read_memory_compressed();

    inline void command_kernel_write();
    inline void command_kernel_read();

    inline void command_take_screenshot();

    inline void command_upload_memory();

    inline void command_server_status();
    inline void command_server_version();

    inline void command_get_data_buffer_size();

    inline void command_read_file();
    inline void command_read_directory();
    inline void command_replace_file();

    inline void command_get_code_handler_address();
    inline void command_read_threads();
    inline void command_account_identifier();
    inline void command_write_screen();
    inline void command_follow_pointer();
    inline void command_remote_procedure_call();
    inline void command_get_symbol();
    inline void command_memory_search_32();
    inline void command_advanced_memory_search();
    inline void command_execute_assembly();

    inline void command_pause_console();
    inline void command_resume_console();
    inline void command_is_console_paused();

    inline void command_get_os_version();

    inline void command_set_data_breakpoint();
    inline void command_set_instruction_breakpoint();
    inline void command_toggle_breakpoint();
    inline void command_remove_all_breakpoints();

    inline void command_poke_registers();
    inline void command_get_stack_trace();
    inline void commmand_get_entry_point_address();
    inline void command_run_kernel_copy_service();
    inline void command_iosu_hax_read_file();
    inline void command_get_version_hash();

    inline void command_persist_assembly();
    inline void command_clear_assembly();
};