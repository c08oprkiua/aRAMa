#include "shared_vars.h"

class CommandHandler: public CommandIO
{
public:

    void command_write_8();
    void command_write_16();
    void command_write_32();

    void command_read_memory();
    void command_read_memory_kernel();
    void command_memory_search_32();
    void command_advanced_memory_search();

    void command_validate_address_range();
    void command_disassemble_range();
    void command_memory_disassemble();
    void command_read_memory_compressed();

    void command_kernel_write();
    void command_kernel_read();

    void command_take_screenshot();

    void command_upload_memory();

    void command_server_status();
    void command_server_version();

    void command_get_data_buffer_size();

    void command_read_file();
    void command_read_directory();
    void command_replace_file();

    void command_get_code_handler_address(); // Depreciate?
    void command_read_threads();
    void command_account_identifier();
    void command_write_screen();
    void command_follow_pointer();
    void command_remote_procedure_call();
    void command_get_symbol();
    void command_memory_search_32();
    void command_advanced_memory_search();
    void command_execute_assembly();

    void command_pause_console();
    void command_resume_console();
    void command_is_console_paused();

    void command_get_os_version();

    void command_set_data_breakpoint();
    void command_set_instruction_breakpoint();
    void command_toggle_breakpoint();
    void command_remove_all_breakpoints();

    void command_poke_registers();
    void command_get_stack_trace();
    void commmand_get_entry_point_address();
    void command_run_kernel_copy_service();
    void command_iosu_hax_read_file();
    void command_get_version_hash();

    void command_persist_assembly();
    void command_clear_assembly();
};