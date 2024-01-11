#include "../command_handler.h"

void CommandHandler::command_pause_console(){
	writeConsoleState(PAUSED);
};
void CommandHandler::command_resume_console(){
	writeConsoleState(RUNNING);
};
void CommandHandler::command_is_console_paused(){
	bool paused = isConsolePaused();
	log_printf("Paused: %d\n", paused);
	ret = sendByte((unsigned char)paused);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendByte (sending paused console status)")
};