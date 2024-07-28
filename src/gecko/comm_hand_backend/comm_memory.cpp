#include "../command_handler.h"

#define ONLY_ZEROS_READ 0xB0
#define NON_ZEROS_READ 0xBD

void CommandHandler::command_read_memory()
{
	const uint8_t *startingAddress, *endingAddress;
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	startingAddress = ((const uint8_t **)buffer)[0];
	endingAddress = ((const uint8_t **)buffer)[1];
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
	const uint8_t *startingAddress, *endingAddress, *useKernRead;
	ret = recvwait(3 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (receiving data)");

	int bufferIndex = 0;
	startingAddress = ((const uint8_t **)buffer)[bufferIndex++];
	endingAddress = ((const uint8_t **)buffer)[bufferIndex++];
	useKernRead = ((const uint8_t **)buffer)[bufferIndex];

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
	const uint8_t *startingAddress, *endingAddress, *useKernRead;
	ret = recvwait(3 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (receiving data)")

	int bufferIndex = 0;
	startingAddress = ((const uint8_t **)buffer)[bufferIndex++];
	endingAddress = ((const uint8_t **)buffer)[bufferIndex++];
	useKernRead = ((const uint8_t **)buffer)[bufferIndex];

	while (startingAddress != endingAddress)
	{
		WHBLogPrintf("Reading memory from %08x to %08x with kernel %i\n", startingAddress, endingAddress,
				   useKernRead);

		uint32_t length = (uint32_t)(endingAddress - startingAddress);

		// Do not smash the buffer
		if (length > DATA_BUFFER_SIZE)
		{
			length = DATA_BUFFER_SIZE;
		}

		// Figure out if all bytes are zero to possibly avoid sending them
		WHBLogPrint("Checking for all zero bytes...\n");
		uint32_t rangeIterationIndex = 0;
		for (; rangeIterationIndex < length; rangeIterationIndex++)
		{
			int character = useKernRead ? readKernelMemory(startingAddress + rangeIterationIndex)
										: startingAddress[rangeIterationIndex];
			if (character != 0)
			{
				break;
			}
		}

		WHBLogPrint("Preparing to send...\n");
		if (rangeIterationIndex == length)
		{
			// No need to send all zero bytes for performance
			WHBLogPrint("All zero...\n");
			ret = sendByte(bss, clientfd, ONLY_ZEROS_READ);
			ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (only zero bytes read byte)")
			WHBLogPrint("Sent!\n");
		}
		else
		{
			// Send the real bytes now
			WHBLogPrint("Real bytes...\n");
			buffer[0] = NON_ZEROS_READ;

			if (useKernRead)
			{
				// kernelCopy(buffer + 1, (uint8_t *) startingAddress, length);
				for (uint32_t offset = 0; offset < length; offset += sizeof(int))
				{
					*((int *)(buffer + 1) + offset / sizeof(int)) = readKernelMemory(
						startingAddress + offset);
					WHBLogPrintf("Offset: %x\n", offset);
				}

				WHBLogPrint("Done kernel reading!\n");
			}
			else
			{
				WHBLogPrint("Memory copying...\n");
				memcpy(buffer + 1, startingAddress, length);
				WHBLogPrint("Done copying!\n");
			}

			WHBLogPrint("Sending everything...\n");
			ret = sendwait(bss, clientfd, buffer, length + 1);
			ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (read bytes buffer)")
			WHBLogPrint("Sent!\n");
		}

		startingAddress += length;
	}

	WHBLogPrint("Done reading...\n");
	*/
};

void CommandHandler::command_memory_search_32(){
	ret = recvwait(sizeof(int) * 3);
	CHECK_ERROR(ret < 0);
	int address = ((int *)buffer)[0];
	int value = ((int *)buffer)[1];
	int length = ((int *)buffer)[2];
	int index;
	int foundAddress = 0;
	for (index = address; index < address + length; index += sizeof(int))
	{
		if (*(int *)index == value)
		{
			foundAddress = index;
			break;
		}
	}

	((int *)buffer)[0] = foundAddress;
	ret = sendwait(sizeof(int));
	CHECK_ERROR(ret < 0)
};

void CommandHandler::command_advanced_memory_search(){
	// Receive the initial data
	ret = recvwait(6 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (memory search information)")
	int bufferIndex = 0;
	int startingAddress = ((int *)buffer)[bufferIndex++];
	int length = ((int *)buffer)[bufferIndex++];
	int kernelRead = ((int *)buffer)[bufferIndex++];
	int resultsLimit = ((int *)buffer)[bufferIndex++];
	int aligned = ((int *)buffer)[bufferIndex++];
	int searchBytesCount = ((int *)buffer)[bufferIndex];

	// Receive the search bytes
	char searchBytes[searchBytesCount];
	ret = recvwait_buffer((uint8_t *)searchBytes, searchBytesCount);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (memory search bytes)")

	int iterationIncrement = aligned ? searchBytesCount : 1;
	int searchBytesOccurrences = 0;

	// Perform the bytes search and collect the results
	for (int currentAddress = startingAddress;
		 currentAddress < startingAddress + length;
		 currentAddress += iterationIncrement)
	{

		int comparisonResult;

		if (kernelRead){
			comparisonResult = kernelMemoryCompare((char *)currentAddress,
												   searchBytes, searchBytesCount);
		}
		else {
			comparisonResult = memoryCompare((void *)currentAddress,
											 searchBytes, (size_t)searchBytesCount);
		}
		if (comparisonResult == 0){
			// Search bytes have been found, add the addresses to the return buffer
			((int *)buffer)[1 + searchBytesOccurrences] = currentAddress;
			searchBytesOccurrences++;

			if ((resultsLimit == searchBytesOccurrences) || (searchBytesOccurrences == ((DATA_BUFFER_SIZE / 4) - 1)))
			{
				// We bail out
				break;
			}
		}
	}

	((int *)buffer)[0] = searchBytesOccurrences * 4;
	ret = sendwait(4 + (searchBytesOccurrences * 4));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (Sending search bytes occurrences)")
};

void CommandHandler::command_read_memory_compressed(){
	// Receive the starting address and length 
/*	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	int startingAddress = ((int *)buffer)[0];
	uint32_t inputLength = ((uint32_t *)buffer)[1];

	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	// Initialize the stream struct
	ret = deflateInit(&stream, Z_BEST_COMPRESSION);
	ASSERT_INTEGER(ret, Z_OK, "deflateInit")

	// Supply the data
	stream.avail_in = inputLength;
	stream.next_in = (Bytef *)startingAddress;
	stream.avail_out = DATA_BUFFER_SIZE;
	void *outputBuffer = (void *)(&buffer + 4);
	stream.next_out = (Bytef *)outputBuffer;

	// Deflate
	ret = deflate(&stream, Z_FINISH);
	ASSERT_INTEGER(ret, Z_OK, "deflate");

	// Finish
	ret = deflateEnd(&stream);
	ASSERT_INTEGER(ret, Z_OK, "deflateEnd");

	// Send the compressed buffer size and content
	int deflatedSize = stream.total_out;
	((int *)buffer)[0] = deflatedSize;
	ret = sendwait(bss, clientfd, buffer, 4 + deflatedSize);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Compressed data)")

	break;
	* /

	// https://www.gamedev.net/resources/_/technical/game-programming/in-memory-data-compression-and-decompression-r2279
	/*

	// Setup compressed buffer
	uint32_t compressedBufferSize = length * 2;
	void *compressedBuffer = (void *) OSAllocFromSystem(compressedBufferSize, 0x4);
	ASSERT_ALLOCATED(compressedBuffer, "Compressed buffer")

	uint32_t zlib_handle;
	OSDynLoad_Acquire("zlib125.rpl", (uint32_t *) &zlib_handle);
	int (*compress2)(char *, int *, const char *, int, int);
	OSDynLoad_FindExport((uint32_t) zlib_handle, 0, "compress2", &compress2);

	int destinationBufferSize;
	int status = compress2((char *) compressedBuffer, &destinationBufferSize,
						   (const char *) rawBuffer, length, Z_DEFAULT_COMPRESSION);

	ret = sendwait(bss, clientfd, &status, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (status)")

	if (status == Z_OK) {
		// Send the compressed buffer size and content
		((int *) buffer)[0] = destinationBufferSize;
		memcpy(buffer + 4, compressedBuffer, destinationBufferSize);

		ret = sendwait(bss, clientfd, buffer, 4 + destinationBufferSize);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Compressed data)")
	}

	free(rawBuffer);
	OSFreeToSystem(compressedBuffer);

	break;*/
};

void CommandHandler::command_upload_memory(){
	// Receive the starting and ending addresses
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	uint8_t *currentAddress = ((uint8_t **)buffer)[0];
	uint8_t *endAddress = ((uint8_t **)buffer)[1];

	while (currentAddress != endAddress)
	{
		int length;

		length = (int)(endAddress - currentAddress);
		if (length > DATA_BUFFER_SIZE)
		{
			length = DATA_BUFFER_SIZE;
		}

		ret = recvwait(length);
		CHECK_ERROR(ret < 0)
		GeckoKernelCopyData(currentAddress, buffer, (uint32_t) length);

		currentAddress += length;
	}
};