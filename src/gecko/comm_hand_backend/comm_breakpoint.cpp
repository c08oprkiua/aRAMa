#include "../command_handler.h"
#include "./tcpgecko/hardware_breakpoints.h"
#include "./tcpgecko/software_breakpoints.h"

inline void CommandHandler::command_set_data_breakpoint(){
	// Read the data from the client
	ret = recvwait(sizeof(int) + sizeof(bool) * 2);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (data breakpoint)");

	// Parse the data and set the breakpoint
	int bufferIndex = 0;
	unsigned int address = ((unsigned int *)buffer)[bufferIndex];
	bufferIndex += sizeof(int);
	bool read = buffer[bufferIndex];
	bufferIndex += sizeof(bool);
	bool write = buffer[bufferIndex];
	bufferIndex += sizeof(bool);
	SetDataBreakpoint(address, read, write);
	// setDataBreakpoint(address, read, write);
};

inline void CommandHandler::command_set_instruction_breakpoint(){
	// Read the address
	ret = recvwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (instruction breakpoint)");

	// Parse the address and set the breakpoint
	unsigned int address = ((unsigned int *)buffer)[0];
	setInstructionBreakpoint(address);
};

inline void CommandHandler::command_toggle_breakpoint(){
	// Read the address
	ret = recvwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (toggle breakpoint)");
	uint32_t address = ((unsigned int *)buffer)[0];

	struct Breakpoint *breakpoint = getBreakpoint(address, GENERAL_BREAKPOINTS_COUNT);

	if (breakpoint != NULL)
	{
		breakpoint = removeBreakpoint(breakpoint);
	}
	else
	{
		breakpoint = allocateBreakpoint();

		if (breakpoint != NULL)
		{
			breakpoint = setBreakpoint(breakpoint, address);
		}
	}
};

inline void CommandHandler::command_remove_all_breakpoints(){
	//removeAllBreakpoints();
	for (int index = 0; index < GENERAL_BREAKPOINTS_COUNT; index++) {
		struct Breakpoint *breakpoint = &breakpoints[index];
		if (breakpoint->address != 0) {
			removeBreakpoint(breakpoint);
		}
	}
};

