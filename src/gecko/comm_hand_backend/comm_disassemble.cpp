#include "../command_handler.h"


void CommandHandler::command_disassemble_range(){
	// Receive the starting, ending address and the disassembler options
	ret = recvwait(4 + 4 + 4);
	CHECK_ERROR(ret < 0)
	void *startingAddress = ((void **)buffer)[0];
	void *endingAddress = ((void **)buffer)[1];
	int disassemblerOptions = ((int *)buffer)[2];

	// Disassemble
	DisassemblePPCRange(startingAddress, endingAddress, formatDisassembled, OSGetSymbolName,
						(DisassemblePPCFlags)disassemblerOptions);

	// Send the disassembler buffer size
	int length = DISASSEMBLER_BUFFER_SIZE;
	ret = sendwait_buffer(&length, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (disassembler buffer size)")

	// Send the data
	ret = sendwait_buffer(disassemblerBufferPointer, length);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (disassembler buffer)")

	// Place the pointer back to the beginning
	disassemblerBuffer = (char *)disassemblerBufferPointer;
}

void CommandHandler::command_memory_disassemble(){
	// Receive the starting address, ending address and disassembler options
	ret = recvwait(sizeof(int) * 3);
	CHECK_ERROR(ret < 0)
	int startingAddress = ((int *)buffer)[0];
	int endingAddress = ((int *)buffer)[1];
	int disassemblerOptions = ((int *)buffer)[2];

	uint32_t currentAddress = startingAddress;
	int bufferSize = PPC_DISASM_MAX_BUFFER;
	int integerSize = 4;

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
													 (DisassemblePPCFlags)disassemblerOptions);

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

		int bytesToSend = currentIntegerIndex * integerSize;
		ret = sendwait_buffer((unsigned char *)&bytesToSend, sizeof(int));
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Buffer size)");

		// VALUE(4)|STATUS(4)|LENGTH(4)|DISASSEMBLED(LENGTH)
		ret = sendwait(bytesToSend);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Buffer)");
	}

	int bytesToSend = 0;
	ret = sendwait_buffer((unsigned char *)&bytesToSend, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (No more bytes)")
};