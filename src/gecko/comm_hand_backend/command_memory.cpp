#include "../command_handler.h"


void CommandHandler::command_read_memory()
{
	const unsigned char *startingAddress, *endingAddress;
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	startingAddress = ((const unsigned char **)buffer)[0];
	endingAddress = ((const unsigned char **)buffer)[1];
	while (startingAddress != endingAddress)
	{
		int length = (int)(endingAddress - startingAddress);
		// Do not smash the buffer
		if (length > DATA_BUFFER_SIZE)
		{
			length = DATA_BUFFER_SIZE;
		}
		// Figure out if all bytes are zero to possibly avoid sending them
		int rangeIterationIndex = 0;
		for (; rangeIterationIndex < length; rangeIterationIndex++)
		{
			int character = startingAddress[rangeIterationIndex];
			if (character != 0)
			{
				break;
			}
		}
		if (rangeIterationIndex == length)
		{
			// No need to send all zero bytes for performance
			ret = sendByte(ONLY_ZEROS_READ);
			CHECK_ERROR(ret < 0)
		}
		else
		{
			// TODO Compression of ptr, sending of status, compressed size and data, length: 1 + 4 + len(data)
			buffer[0] = NON_ZEROS_READ;
			memcpy(buffer + 1, startingAddress, length);
			ret = sendwait(length + 1);
			CHECK_ERROR(ret < 0)
		}
		/* 	No exit condition.
		We reconnect client-sided instead as a hacky work-around
		 to gain a little more performance by avoiding the very rare search canceling
		 */
		startingAddress += length;
	}
};

void CommandHandler::command_read_memory_kernel(){
	const unsigned char *startingAddress, *endingAddress, *useKernRead;
	ret = recvwait(3 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (receiving data)");

	int bufferIndex = 0;
	startingAddress = ((const unsigned char **)buffer)[bufferIndex++];
	endingAddress = ((const unsigned char **)buffer)[bufferIndex++];
	useKernRead = ((const unsigned char **)buffer)[bufferIndex];

	while (startingAddress != endingAddress)
	{
		int length = (int)(endingAddress - startingAddress);

		// Do not smash the buffer
		if (length > DATA_BUFFER_SIZE)
		{
			length = DATA_BUFFER_SIZE;
		}

		// Figure out if all bytes are zero to possibly avoid sending them
		int rangeIterationIndex = 0;
		for (; rangeIterationIndex < length; rangeIterationIndex++)
		{
			int character = useKernRead ? readKernelMemory(startingAddress + rangeIterationIndex)
										: startingAddress[rangeIterationIndex];
			if (character != 0)
			{
				break;
			}
		}

		if (rangeIterationIndex == length)
		{
			// No need to send all zero bytes for performance
			ret = sendByte(ONLY_ZEROS_READ);
			CHECK_ERROR(ret < 0)
		}
		else
		{
			buffer[0] = NON_ZEROS_READ;

			if (useKernRead)
			{
				for (int offset = 0; offset < length; offset += 4)
				{
					*((int *)(buffer + 1) + offset / 4) = readKernelMemory(startingAddress + offset);
				}
			}
			else
			{
				memcpy(buffer + 1, startingAddress, length);
			}

			ret = sendwait(length + 1);
			CHECK_ERROR(ret < 0)
		}

		/* 	No exit condition.
			We reconnect client-sided instead as a hacky work-around
			 to gain a little more performance by avoiding the very rare search canceling
		 */

		startingAddress += length;
	}

	/*
	const unsigned char *startingAddress, *endingAddress, *useKernRead;
	ret = recvwait(3 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (receiving data)")

	int bufferIndex = 0;
	startingAddress = ((const unsigned char **)buffer)[bufferIndex++];
	endingAddress = ((const unsigned char **)buffer)[bufferIndex++];
	useKernRead = ((const unsigned char **)buffer)[bufferIndex];

	while (startingAddress != endingAddress)
	{
		log_printf("Reading memory from %08x to %08x with kernel %i\n", startingAddress, endingAddress,
				   useKernRead);

		unsigned int length = (unsigned int)(endingAddress - startingAddress);

		// Do not smash the buffer
		if (length > DATA_BUFFER_SIZE)
		{
			length = DATA_BUFFER_SIZE;
		}

		// Figure out if all bytes are zero to possibly avoid sending them
		log_print("Checking for all zero bytes...\n");
		unsigned int rangeIterationIndex = 0;
		for (; rangeIterationIndex < length; rangeIterationIndex++)
		{
			int character = useKernRead ? readKernelMemory(startingAddress + rangeIterationIndex)
										: startingAddress[rangeIterationIndex];
			if (character != 0)
			{
				break;
			}
		}

		log_print("Preparing to send...\n");
		if (rangeIterationIndex == length)
		{
			// No need to send all zero bytes for performance
			log_print("All zero...\n");
			ret = sendByte(bss, clientfd, ONLY_ZEROS_READ);
			ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (only zero bytes read byte)")
			log_print("Sent!\n");
		}
		else
		{
			// Send the real bytes now
			log_print("Real bytes...\n");
			buffer[0] = NON_ZEROS_READ;

			if (useKernRead)
			{
				// kernelCopy(buffer + 1, (unsigned char *) startingAddress, length);
				for (unsigned int offset = 0; offset < length; offset += sizeof(int))
				{
					*((int *)(buffer + 1) + offset / sizeof(int)) = readKernelMemory(
						startingAddress + offset);
					log_printf("Offset: %x\n", offset);
				}

				log_print("Done kernel reading!\n");
			}
			else
			{
				log_print("Memory copying...\n");
				memcpy(buffer + 1, startingAddress, length);
				log_print("Done copying!\n");
			}

			log_print("Sending everything...\n");
			ret = sendwait(bss, clientfd, buffer, length + 1);
			ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (read bytes buffer)")
			log_print("Sent!\n");
		}

		startingAddress += length;
	}

	log_print("Done reading...\n");
	*/
};

void CommandHandler::command_memory_search_32(){

};
void CommandHandler::command_advanced_memory_search(){

};