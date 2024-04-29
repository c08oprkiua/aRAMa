#include "../command_handler.h"


void CommandHandler::command_set_data_breakpoint(){
	// Read the data from the client
	ret = recvwait(sizeof(int) + sizeof(bool) * 2);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (data breakpoint)");

	// Parse the data and set the breakpoint
	int bufferIndex = 0;
	uint32_t address = ((uint32_t *)buffer)[bufferIndex];
	bufferIndex += sizeof(int);
	bool read = buffer[bufferIndex];
	bufferIndex += sizeof(bool);
	bool write = buffer[bufferIndex];
	bufferIndex += sizeof(bool);
	SetDataBreakpoint(address, read, write);
	// setDataBreakpoint(address, read, write);
};

void CommandHandler::command_set_instruction_breakpoint(){
	// Read the address
	ret = recvwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (instruction breakpoint)");

	// Parse the address and set the breakpoint
	uint32_t address = ((uint32_t *)buffer)[0];
	setInstructionBreakpoint(address);
};

void CommandHandler::command_toggle_breakpoint(){
	// Read the address
	ret = recvwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (toggle breakpoint)");
	uint32_t address = ((uint32_t *)buffer)[0];

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

void CommandHandler::command_remove_all_breakpoints(){
	//removeAllBreakpoints();
	for (int index = 0; index < GENERAL_BREAKPOINTS_COUNT; index++) {
		struct Breakpoint *breakpoint = &breakpoints[index];
		if (breakpoint->address != 0) {
			removeBreakpoint(breakpoint);
		}
	}
};

void removeBreakpoint(Breakpoint bp){

}