#include "../command_handler.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <coreinit/debug.h>

#include "core/log_and_noti.h"

char *disassemblerBuffer;
void *disassemblerBufferPointer;

#define DISASSEMBLER_BUFFER_SIZE 0x1024
//this value is a patchwork
#define PPC_DISASM_MAX_BUFFER DISASSEMBLER_BUFFER_SIZE

void formatDisassembled(const char *format, ...) {
	if (!disassemblerBuffer) {
		disassemblerBuffer = (char *) malloc(DISASSEMBLER_BUFFER_SIZE);
		ASSERT_ALLOCATED(disassemblerBuffer, "Disassembler buffer")
		disassemblerBufferPointer = disassemblerBuffer;
	}

	va_list variableArguments;
	va_start(variableArguments, format);
	char *temporaryBuffer;
	int printedBytesCount = vasprintf(&temporaryBuffer, format, variableArguments);
	ASSERT_ALLOCATED(temporaryBuffer, "Temporary buffer")
	ASSERT_MINIMUM_HOLDS(printedBytesCount, 1, "Printed bytes count")
	va_end(variableArguments);

	// Do not smash the buffer
	long projectedSize = disassemblerBuffer - (char *) disassemblerBufferPointer + printedBytesCount;
	if (projectedSize < DISASSEMBLER_BUFFER_SIZE) {
		memcpy(disassemblerBuffer, temporaryBuffer, printedBytesCount);
		disassemblerBuffer += printedBytesCount;
	}

	free(temporaryBuffer);
}

int roundUpToAligned(int number) {
	return (number + 3) & ~0x03;
}

void CommandHandler::command_disassemble_range(){
	// Receive the starting, ending address and the disassembler options
	ret = recvwait(4 + 4 + 4);
	void *startingAddress, *endingAddress;
	DisassemblePPCFlags disassemblerOptions;
	int length;
	CHECK_ERROR(ret < 0);
	startingAddress = ((void **)buffer)[0];
	endingAddress = ((void **)buffer)[1];
	disassemblerOptions = ((DisassemblePPCFlags *)buffer)[2];

	// Disassemble
	DisassemblePPCRange(startingAddress, endingAddress, formatDisassembled, OSGetSymbolName,
						disassemblerOptions);

	// Send the disassembler buffer size
	length = DISASSEMBLER_BUFFER_SIZE;
	ret = sendwait_buffer((uint8_t *)length, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (disassembler buffer size)")

	// Send the data
	ret = sendwait_buffer((uint8_t *)disassemblerBufferPointer, length);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (disassembler buffer)")

	// Place the pointer back to the beginning
	disassemblerBuffer = (char *)disassemblerBufferPointer;

	return;

	error:
	ret = -1;
	return;
}

void CommandHandler::command_memory_disassemble(){
	// Receive the starting address, ending address and disassembler options
	ret = recvwait(sizeof(int) * 3);
	DisassemblePPCFlags disassemblerOptions;
	int bufferSize, integerSize, bytesToSend;
	uint32_t startingAddress,endingAddress,currentAddress;
	CHECK_ERROR(ret < 0)
	startingAddress = ((uint32_t *)buffer)[0];
	endingAddress = ((uint32_t *)buffer)[1];
	disassemblerOptions = ((DisassemblePPCFlags *)buffer)[2];

	currentAddress = startingAddress;
	bufferSize = PPC_DISASM_MAX_BUFFER;
	integerSize = 4;

	// Disassemble everything
	while (currentAddress < endingAddress)
	{
		int currentIntegerIndex = 0;

		while ((currentIntegerIndex < (DATA_BUFFER_SIZE / integerSize)) && (currentAddress < endingAddress))
		{
			int value = *(int *)currentAddress;
			((int *)buffer)[currentIntegerIndex++] = value;
			char *opCodeBuffer = (char *)malloc(bufferSize);
			bool status = (bool)DisassemblePPCOpcode(&currentAddress, opCodeBuffer, bufferSize,
													 OSGetSymbolName,
													 disassemblerOptions);

			((int *)buffer)[currentIntegerIndex++] = status;

			if (status == 1)
			{
				// Send the length of the opCode buffer string
				int length = strlen(opCodeBuffer);
				((int *)buffer)[currentIntegerIndex++] = length;

				// Send the opCode buffer itself
				memcpy(buffer + (currentIntegerIndex * integerSize), opCodeBuffer, length);
				currentIntegerIndex += (roundUpToAligned(length) / integerSize);
			}

			free(opCodeBuffer);
			currentAddress += integerSize;
		}

		bytesToSend = currentIntegerIndex * integerSize;
		ret = sendwait_buffer((uint8_t *)&bytesToSend, sizeof(int));
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Buffer size)");

		// VALUE(4)|STATUS(4)|LENGTH(4)|DISASSEMBLED(LENGTH)
		ret = sendwait(bytesToSend);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Buffer)");
	}

	bytesToSend = 0;
	ret = sendwait_buffer((uint8_t *)&bytesToSend, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (No more bytes)");

	return;

	error:
	ret = -1;
	return;
};