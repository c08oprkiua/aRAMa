#include "gecko/command_handler.h"
#include "arama.h"
#include <kernel/kernel.h>
#include <coreinit/debug.h>
#include <coreinit/memorymap.h>
#include <coreinit/cache.h>

void CommandHandler::command_execute_assembly(){
	receiveString((uint8_t *)buffer, DATA_BUFFER_SIZE);
	if (!singletons.config->get_code_handler()){
		//something something "this is illegal, you know"
	}

	singletons.code_handler->executeAssembly();
	
	//executeAssembly(buffer, DATA_BUFFER_SIZE);
};

void CommandHandler::command_persist_assembly(){
	uint32_t length = receiveString((uint8_t *)buffer, DATA_BUFFER_SIZE);
	if (!singletons.config->get_code_handler()){
		//something something "this is illegal, you know"
	}
	singletons.code_handler->persistAssembly(length, buffer);
};

void CommandHandler::command_clear_assembly(){
	singletons.code_handler->clear();
};