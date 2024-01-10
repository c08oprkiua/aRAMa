#include "command_handler.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/filesystem.h>
#include <coreinit/thread.h> 
#include <coreinit/memory.h>
#include <coreinit/dynload.h>
#include <coreinit/mcp.h>

#include <nn/act.h>

#include <nsysnet/socket.h> 

#include <gx2/event.h>


#include <kernel/kernel.h>

#include "../arama.h"
#include "../tcpgecko/address.h"
#include "../tcpgecko/assertions.h"
#include "../tcpgecko/disassembler.h"
#include "../tcpgecko/threads.h"
#include "../tcpgecko/kernel.h"

#define CHECK_ERROR(cond)     \
	if (cond)                 \
	{                         \
		line = __LINE__; \
		goto error;       \
	}

#define errno2 (*__gh_errno_ptr())
#define MSG_DONT_WAIT 32
#define E_WOULD_BLOCK 6
#define WRITE_SCREEN_MESSAGE_BUFFER_SIZE 100
#define SERVER_VERSION "06/03/2017"

#define ONLY_ZEROS_READ 0xB0
#define NON_ZEROS_READ 0xBD
#define VERSION_HASH 0x7FB223

#define FS_MAX_LOCALPATH_SIZE           511
#define FS_MAX_MOUNTPATH_SIZE           128
#define FS_MAX_FULLPATH_SIZE            (FS_MAX_LOCALPATH_SIZE + FS_MAX_MOUNTPATH_SIZE)

#define DISASSEMBLER_BUFFER_SIZE 0x1024

// The time the producer and consumer wait while there is nothing to do
#define WAITING_TIME_MILLISECONDS 1



void CommandHandler::command_write_8()
{
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0);

	char *destinationAddress = ((char **)buffer)[0];
	*destinationAddress = buffer[7];
	DCFlushRange(destinationAddress, 1);
	return;
};

void CommandHandler::command_write_16()
{
	short *destinationAddress;
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	destinationAddress = ((short **)buffer)[0];
	*destinationAddress = ((short *)buffer)[3];
	DCFlushRange(destinationAddress, 2);
	return;
};

void CommandHandler::command_write_32()
{
	int destinationAddress, value;
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	destinationAddress = ((int *)buffer)[0];
	value = ((int *)buffer)[1];

	GeckoKernelCopyData((unsigned char *) destinationAddress, (unsigned char *) &value, sizeof(int));
	
};

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

void CommandHandler::command_validate_address_range(){
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	// Retrieve the data
	int startingAddress = ((int *)buffer)[0];
	int endingAddress = ((int *)buffer)[1];

	int isAddressRangeValid = validateAddressRange(startingAddress, endingAddress);

	sendByte((unsigned char)isAddressRangeValid);
};

void CommandHandler::command_disassemble_range(){
	// Receive the starting, ending address and the disassembler options
	ret = recvwait( 4 + 4 + 4);
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

void CommandHandler::command_read_memory_compressed(){
	// Receive the starting address and length 
/*	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	int startingAddress = ((int *)buffer)[0];
	unsigned int inputLength = ((unsigned int *)buffer)[1];

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
	unsigned int compressedBufferSize = length * 2;
	void *compressedBuffer = (void *) OSAllocFromSystem(compressedBufferSize, 0x4);
	ASSERT_ALLOCATED(compressedBuffer, "Compressed buffer")

	unsigned int zlib_handle;
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

void CommandHandler::command_kernel_write(){
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	void *address = ((void **)buffer)[0];
	void *value = ((void **)buffer)[1];

	writeKernelMemory(address, (uint32_t)value);
};

void CommandHandler::command_kernel_read(){
	ret = recvwait(sizeof(int));
	CHECK_ERROR(ret < 0)

	void *address = ((void **)buffer)[0];
	void *value = (void *)readKernelMemory(address);

	*(void **)buffer = value;
	sendwait(sizeof(int));
};

void CommandHandler::command_take_screenshot(){
	// Tell the hook to dump the screen shot now
	shouldTakeScreenShot = true;

	// Tell the client the size of the upcoming image
	ret = sendwait_buffer((unsigned char *)&totalImageSize, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (total image size)")

	// Keep sending the image data
	while (remainingImageSize > 0)
	{
		int bufferPosition = 0;

		// Fill the buffer till it is full
		while (bufferPosition <= DATA_BUFFER_SIZE)
		{
			// Wait for data to be available
			while (bufferedImageSize == 0)
			{
				usleep(WAITING_TIME_MILLISECONDS);
			}

			memcpy(buffer + bufferPosition, bufferedImageData, bufferedImageSize);
			bufferPosition += bufferedImageSize;
			bufferedImageSize = 0;
		}

		// Send the size of the current chunk
		ret = sendwait_buffer((unsigned char *)&bufferPosition, sizeof(int));
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image data chunk size)")

		// Send the image data itself
		ret = sendwait(bufferPosition);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image data)")
	}

	/*GX2ColorBuffer colorBuffer;
	// TODO Initialize colorBuffer!
	GX2Surface surface = colorBuffer.surface;
	void *image_data = surface.image_data;
	uint32_t image_size = surface.image_size;

	// Send the image size so that the client knows how much to read
	ret = sendwait(bss, clientfd, (unsigned char *) &image_size, sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image size)")

	unsigned int imageBytesSent = 0;
	while (imageBytesSent < image_size) {
		int length = image_size - imageBytesSent;

		// Do not smash the buffer
		if (length > DATA_BUFFER_SIZE) {
			length = DATA_BUFFER_SIZE;
		}

		// Send the image bytes
		memcpy(buffer, image_data, length);
		ret = sendwait(bss, clientfd, buffer, length);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (image bytes)")

		imageBytesSent += length;
	}*/
};

void CommandHandler::command_upload_memory(){
	// Receive the starting and ending addresses
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)
	unsigned char *currentAddress = ((unsigned char **)buffer)[0];
	unsigned char *endAddress = ((unsigned char **)buffer)[1];

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
		GeckoKernelCopyData(currentAddress, buffer, (unsigned int) length);

		currentAddress += length;
	}
};

void CommandHandler::command_server_status()
{
	ret = sendByte(1);
	CHECK_ERROR(ret < 0)
	return;
};

void CommandHandler::command_server_version(){
	char versionBuffer[50];
	strcpy(versionBuffer, SERVER_VERSION);
	int versionLength = strlen(versionBuffer);
	((int *)buffer)[0] = versionLength;
	memcpy(buffer + sizeof(int), versionBuffer, versionLength);

	// Send the length and the version string
	ret = sendwait(sizeof(int) + versionLength);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (server version)");
};

void CommandHandler::command_get_data_buffer_size(){
	log_printf("COMMAND_GET_DATA_BUFFER_SIZE...\n");
	((int *)buffer)[0] = DATA_BUFFER_SIZE;
	log_printf("Sending buffer size...\n");
	ret = sendwait(sizeof(int));
	log_printf("Sent: %i\n", ret);
	CHECK_ERROR(ret < 0)
};

void CommandHandler::command_read_file(){
	char file_path[FS_MAX_FULLPATH_SIZE] = {0};
	receiveString((unsigned char *)file_path, FS_MAX_FULLPATH_SIZE);

	considerInitializingFileSystem();

	int handle;
	int status = FSOpenFile(client, commandBlock, file_path, "r", &handle, FS_ERROR_FLAG_ALL);

	if (status == FS_STATUS_OK)
	{
		// Send the OK status
		((int *)buffer)[0] = status;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (OK status)")

		// Retrieve the file statistics
		FSStat stat;
		ret = FSGetStatFile(client, commandBlock, handle, &stat, FS_ERROR_FLAG_ALL);
		ASSERT_FUNCTION_SUCCEEDED(ret, "FSGetStatFile")

		// Send the total bytes count
		int totalBytes = (int)stat.size;
		((int *)buffer)[0] = totalBytes;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (total bytes)")

		// Allocate the file bytes buffer
		unsigned int file_buffer_size = 0x2000;
		char *fileBuffer = (char *)OSAllocFromSystem(file_buffer_size, FS_IO_BUFFER_ALIGN);
		ASSERT_ALLOCATED(fileBuffer, "File buffer")

		int totalBytesRead = 0;
		while (totalBytesRead < totalBytes)
		{
			int bytesRead = FSReadFile(client, commandBlock, fileBuffer, 1, file_buffer_size,
									   handle, 0, FS_ERROR_FLAG_ALL);
			ASSERT_FUNCTION_SUCCEEDED(bytesRead, "FSReadFile")

			// Send file bytes
			ret = sendwait_buffer((unsigned char *)fileBuffer, bytesRead);
			ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (file buffer)")

			totalBytesRead += bytesRead;
		}

		ret = FSCloseFile(client, commandBlock, handle, FS_ERROR_FLAG_ALL);
		ASSERT_FUNCTION_SUCCEEDED(ret, "FSCloseFile")

		OSFreeToSystem(fileBuffer);
	}
	else
	{
		// Send the error status
		((int *)buffer)[0] = status;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (error status)")
	}
};

void CommandHandler::command_read_directory(){
	char directory_path[FS_MAX_FULLPATH_SIZE] = {0};
	receiveString((unsigned char *)directory_path, FS_MAX_FULLPATH_SIZE);

	considerInitializingFileSystem();

	s32 handle;
	FSDirectoryEntry entry;

	ret = FSOpenDir(client, commandBlock, directory_path, &handle, FS_ERROR_FLAG_ALL);

	if (ret == FS_STATUS_OK)
	{
		// Send the success status
		((int *)buffer)[0] = ret;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (success status)")

		int entrySize = sizeof(FSDirectoryEntry);

		// Read every entry in the given directory
		while (FSReadDir(client, commandBlock, handle, &entry, -1) == FS_STATUS_OK)
		{
			// Let the client know how much data is going to be sent (even though this is constant)
			((int *)buffer)[0] = entrySize;
			ret = sendwait(4);
			ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (data coming)")

			// Send the struct
			ret = sendwait_buffer((unsigned char *)&entry, entrySize);
			ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (directory entry)")
		}

		// No more data will be sent, hence a 0 byte
		((int *)buffer)[0] = 0;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (no more data)")

		// Done, close the directory also
		ret = FSCloseDir(client, commandBlock, handle, FS_ERROR_FLAG_ALL);
		ASSERT_FUNCTION_SUCCEEDED(ret, "FSCloseDir")
	}
	else
	{
		// Send the status
		((int *)buffer)[0] = ret;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (error status)")
	}
};

void CommandHandler::command_replace_file(){
	// TODO FSOpenFile ACCESS_ERROR

	// Receive the file path
	char file_path[FS_MAX_FULLPATH_SIZE] = {0};
	receiveString((unsigned char *)file_path, FS_MAX_FULLPATH_SIZE);

	considerInitializingFileSystem();

	// Create an empty file for writing. Its contents will be erased
	int handle;
	int status = FSOpenFile(client, commandBlock, file_path, "w", &handle, FS_ERROR_FLAG_ALL);

	if (status == FS_STATUS_OK)
	{
		// Send the OK status
		((int *)buffer)[0] = status;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (OK status)")

		// Set the file handle position to the beginning
		ret = FSSetPosFile(client, commandBlock, handle, 0, FS_ERROR_FLAG_ALL);
		ASSERT_FUNCTION_SUCCEEDED(ret, "FSSetPosFile")

		// Allocate the file bytes buffer
		unsigned int file_buffer_size = 0x2000;
		char *fileBuffer = (char *)OSAllocFromSystem(file_buffer_size, FS_IO_BUFFER_ALIGN);
		ASSERT_ALLOCATED(fileBuffer, "File buffer")

		// Send the maximum file buffer size
		ret = sendwait_buffer((unsigned char *)&file_buffer_size, 4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (maximum file buffer size)")

		while (true)
		{
			// Receive the data bytes length
			unsigned int dataLength;
			ret = recvwait_buffer((unsigned char *)&dataLength, 4);
			ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (File bytes length)")
			ASSERT_MAXIMUM_HOLDS(file_buffer_size, dataLength, "File buffer overrun attempted")

			if (dataLength > 0)
			{
				// Receive the data
				ret = recvwait_buffer((unsigned char *)fileBuffer, dataLength);
				ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (File buffer)")

				// Write the data (and advance file handle position implicitly)
				ret = FSWriteFile(client, commandBlock, (uint8_t *)fileBuffer, 1,
								  dataLength, handle, 0, FS_ERROR_FLAG_ALL);
				ASSERT_FUNCTION_SUCCEEDED(ret, "FSWriteFile")
			}
			else
			{
				// Done with receiving the new file
				break;
			}
		}

		// Flush the file back
		// ret = FSFlushFile(client, commandBlock, handle, FS_RET_ALL_ERROR);
		// CHECK_FUNCTION_FAILED(ret, "FSFlushFile")

		// Close the file
		ret = FSCloseFile(client, commandBlock, handle, FS_ERROR_FLAG_ALL);
		ASSERT_FUNCTION_SUCCEEDED(ret, "FSCloseFile")

		// Free the file buffer
		OSFreeToSystem(fileBuffer);
	}
	else
	{
		// Send the status
		((int *)buffer)[0] = status;
		ret = sendwait(4);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (status)")
	}
};

void CommandHandler::command_get_code_handler_address(){
	((int *)buffer)[0] = CODE_HANDLER_INSTALL_ADDRESS;
	ret = sendwait(4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (code handler address)")
} // Depreciate?

void CommandHandler::command_read_threads(){
	struct node *threads = getAllThreads();
	int threadCount = length(threads);
	log_printf("Thread Count: %i\n", threadCount);

	// Send the thread count
	log_print("Sending thread count...\n");
	((int *)buffer)[0] = threadCount;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (thread count)");

	// Send the thread addresses and data
	struct node *currentThread = threads;
	while (currentThread != NULL)
	{
		int data = (int)currentThread->data;
		log_printf("Thread data: %08x\n", data);
		((int *)buffer)[0] = (int)currentThread->data;
		memcpy(buffer + sizeof(int), currentThread->data, THREAD_SIZE);
		log_print("Sending node...\n");
		ret = sendwait(sizeof(int) + THREAD_SIZE);
		ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (thread address and data)")

		currentThread = currentThread->next;
	}

	destroy(threads);
};

void CommandHandler::command_account_identifier(){
	// Acquire the RPL

	/*Unneeded in WUT

	uint32_t nn_act_handle;
	OSDynLoad_Acquire("nn_act.rpl", &nn_act_handle);

	// Acquire the functions via their mangled file names
	int (*nn_act_Initialize)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "Initialize__Q2_2nn3actFv", &nn_act_Initialize);
	ASSERT_ALLOCATED(nn_act_Initialize, "nn_act_Initialize")
	unsigned char (*nn_act_GetSlotNo)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetSlotNo__Q2_2nn3actFv", &nn_act_GetSlotNo);
	ASSERT_ALLOCATED(nn_act_GetSlotNo, "nn_act_GetSlotNo")
	unsigned int (*nn_act_GetPersistentIdEx)(unsigned char);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetPersistentIdEx__Q2_2nn3actFUc", &nn_act_GetPersistentIdEx);
	ASSERT_ALLOCATED(nn_act_GetPersistentIdEx, "nn_act_GetPersistentIdEx")
	int (*nn_act_Finalize)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "Finalize__Q2_2nn3actFv", &nn_act_Finalize);
	ASSERT_ALLOCATED(nn_act_Finalize, "nn_act_Finalize")
	*/

	// Get the identifier
	// ret = nn_act_Initialize();
	ret = (int) nn::act::Initialize();
	// ASSERT_INTEGER(ret, 1, "Initializing account library");

	nn::act::SlotNo slotNumber = nn::act::GetSlotNo();
	nn::act::PersistentId persistentIdentifier = nn::act::GetPersistentIdEx(slotNumber);
	ret = (int) nn::act::Finalize();
	ASSERT_FUNCTION_SUCCEEDED(ret, "nn_act_Finalize");

	// Send it
	ret = sendwait_buffer((unsigned char *)&persistentIdentifier, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (persistent identifier)")
};

void CommandHandler::command_write_screen(){/*
	char message[WRITE_SCREEN_MESSAGE_BUFFER_SIZE];
	ret = recvwait(4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (write screen seconds)")
	int seconds = ((int *)buffer)[0];
	receiveString(bss, clientfd, message, WRITE_SCREEN_MESSAGE_BUFFER_SIZE);
	writeScreen(message, seconds);

	break;*/
};
void CommandHandler::command_follow_pointer(){
	ret = recvwait(sizeof(int) * 2);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (Pointer address and offsets count)")

	// Retrieve the pointer address and amount of offsets
	int baseAddress = ((int *)buffer)[0];
	int offsetsCount = ((int *)buffer)[1];

	// Receive the offsets
	ret = recvwait(offsetsCount * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (offsets)")
	int offsets[offsetsCount];
	int offsetIndex = 0;
	for (; offsetIndex < offsetsCount; offsetIndex++)
	{
		offsets[offsetIndex] = ((int *)buffer)[offsetIndex];
	}

	int destinationAddress = baseAddress;

#define INVALID_ADDRESS -1

	if ((bool)OSIsAddressValid(destinationAddress))
	{
		// Apply pointer offsets
		for (offsetIndex = 0; offsetIndex < offsetsCount; offsetIndex++)
		{
			int pointerValue = *(int *)destinationAddress;
			int offset = offsets[offsetIndex];
			destinationAddress = pointerValue + offset;

			// Validate the pointer address
			bool isValidDestinationAddress = (bool)OSIsAddressValid(destinationAddress);

			// Bail out if invalid
			if (!isValidDestinationAddress)
			{
				destinationAddress = INVALID_ADDRESS;

				break;
			}
		}
	}
	else if (offsetsCount > 0)
	{
		// Following pointers failed
		destinationAddress = INVALID_ADDRESS;
	}

	// Return the destination address
	((int *)buffer)[0] = destinationAddress;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (destination address)")
};
void CommandHandler::command_remote_procedure_call(){
	int r3, r4, r5, r6, r7, r8, r9, r10;

	log_print("Receiving RPC information...\n");
	ret = recvwait(sizeof(int) + 8 * sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "revcwait() Receiving RPC information")
	log_print("RPC information received...\n");

	long long (*function)(int, int, int, int, int, int, int, int);
	function = (long long int (*)(int, int, int, int, int, int, int, int))((void **)buffer)[0];
	r3 = ((int *)buffer)[1];
	r4 = ((int *)buffer)[2];
	r5 = ((int *)buffer)[3];
	r6 = ((int *)buffer)[4];
	r7 = ((int *)buffer)[5];
	r8 = ((int *)buffer)[6];
	r9 = ((int *)buffer)[7];
	r10 = ((int *)buffer)[8];

	log_print("Calling function...\n");
	long long result = function(r3, r4, r5, r6, r7, r8, r9, r10);
	log_printf("Function successfully called with return value: 0x%08x 0x%08x\n", (int)(result >> 32),
			   (int)result);

	log_print("Sending result...\n");
	((long long *)buffer)[0] = result;
	ret = sendwait(sizeof(long long));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait() Sending return value")
	log_print("Result successfully sent...\n");
};

void CommandHandler::command_get_symbol(){
	int size = recvbyte();
	CHECK_ERROR(size < 0)

	ret = recvwait(size);
	CHECK_ERROR(ret < 0)

	/* Identify the RPL name and symbol name */
	char *rplName = (char *)&((int *)buffer)[2];
	char *symbolName = (char *)(&buffer[0] + ((int *)buffer)[1]);

	/* Get the symbol and store it in the buffer */
	OSDynLoad_Module module_handle;
	void *function_address;
	OSDynLoad_Acquire(rplName, &module_handle);

	OSDynLoad_ExportType data = (OSDynLoad_ExportType)recvbyte(bss, clientfd);
	OSDynLoad_FindExport(module_handle, data, symbolName, &function_address);

	((int *)buffer)[0] = (int)function_address;
	ret = sendwait(4);
	CHECK_ERROR(ret < 0)
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
	ret = recvwait_buffer((unsigned char *)searchBytes, searchBytesCount);
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (memory search bytes)")

	int iterationIncrement = aligned ? searchBytesCount : 1;
	int searchBytesOccurrences = 0;

	// Perform the bytes search and collect the results
	for (int currentAddress = startingAddress;
		 currentAddress < startingAddress + length;
		 currentAddress += iterationIncrement)
	{

		int comparisonResult;

		if (kernelRead)
		{
			comparisonResult = kernelMemoryCompare((char *)currentAddress,
												   searchBytes, searchBytesCount);
		}
		else
		{
			comparisonResult = memoryCompare((void *)currentAddress,
											 searchBytes, (size_t)searchBytesCount);
		}
		if (comparisonResult == 0)
		{
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
void CommandHandler::command_execute_assembly(){
	receiveString((unsigned char *)buffer, DATA_BUFFER_SIZE);
	executeAssembly(buffer, DATA_BUFFER_SIZE);
};

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

void CommandHandler::command_get_os_version(){
	MCPSystemVersion version;
	//Idk what to put for handle
	MCP_GetSystemVersion(0, &version);

	((int *)buffer)[0] = version.major;
	((int *)buffer)[1] = version.minor;
	((int *)buffer)[2] = version.patch;
	buffer[3] = version.region;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (OS version)");
};

void CommandHandler::command_set_data_breakpoint(){
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

void CommandHandler::command_set_instruction_breakpoint(){
	// Read the address
	ret = recvwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "recvwait (instruction breakpoint)");

	// Parse the address and set the breakpoint
	unsigned int address = ((unsigned int *)buffer)[0];
	setInstructionBreakpoint(address);
};

void CommandHandler::command_toggle_breakpoint(){
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

void CommandHandler::command_remove_all_breakpoints(){
	removeAllBreakpoints();
};

void CommandHandler::command_poke_registers(){
	log_print("Receiving poke registers data...\n");
	int gprSize = 4 * 32;
	int fprSize = 8 * 32;
	ret = recvwait(gprSize + fprSize);
	log_print("Poking registers...\n");
	memcpy((void *)crashContext.gpr, (const void *)buffer, gprSize);
	memcpy((void *)crashContext.fpr, (const void *)buffer, fprSize);
};

void CommandHandler::command_get_stack_trace(){
	log_print("Getting stack trace...\n");
	struct node *stackTrace = getStackTrace(NULL);
	int stackTraceLength = length(stackTrace);

	// Let the client know the length beforehand
	int bufferIndex = 0;
	((int *)buffer)[bufferIndex++] = stackTraceLength;

	struct node *currentStackTraceElement = stackTrace;
	while (currentStackTraceElement != NULL)
	{
		int address = (int)currentStackTraceElement->data;
		log_printf("Stack trace element address: %08x\n", address);
		((int *)buffer)[bufferIndex++] = (int)currentStackTraceElement->data;

		currentStackTraceElement = currentStackTraceElement->next;
	}

	log_printf("Sending stack trace with length %i\n", stackTraceLength);
	ret = sendwait(sizeof(int) + stackTraceLength);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (stack trace)");
};

void CommandHandler::commmand_get_entry_point_address(){
	uint32_t *entryPointAddress = (uint32_t *)*((uint32_t *)OS_SPECIFICS->addr_OSTitle_main_entry);
	((uint32_t *)buffer)[0] = (uint32_t)entryPointAddress;
	ret = sendwait(sizeof(int));
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (Entry point address)");
};

void CommandHandler::command_run_kernel_copy_service(){
	if (!kernelCopyServiceStarted)
	{
		kernelCopyServiceStarted = true;
		startKernelCopyService();
	}
};

void CommandHandler::command_iosu_hax_read_file(){
	/*log_print("COMMAND_IOSUHAX_READ_FILE");

	// TODO Crashes console on this call
	int returnValue = IOSUHAX_Open(NULL);
	log_print("IOSUHAX_Open Done");
	log_printf("IOSUHAX_Open: %i", returnValue);

	if (returnValue < 0) {
		goto IOSUHAX_OPEN_FAILED;
	}

	int fileSystemFileDescriptor = IOSUHAX_FSA_Open();
	log_printf("IOSUHAX_FSA_Open: %i", fileSystemFileDescriptor);

	if (fileSystemFileDescriptor < 0) {
		goto IOSUHAX_FSA_OPEN_FAILED;
	}

	int fileDescriptor;
	const char *filePath = "/vol/storage_usb/usr/title/0005000e/1010ed00/content/audio/stream/pBGM_GBA_CHEESELAND_F.bfstm";
	returnValue = IOSUHAX_FSA_OpenFile(fileSystemFileDescriptor, filePath, "rb", &fileDescriptor);
	log_printf("IOSUHAX_FSA_OpenFile: %i", returnValue);

	if (returnValue < 0) {
		goto IOSUHAX_OPEN_FILE_FAILED;
	}

	fileStat_s fileStat;
	returnValue = IOSUHAX_FSA_StatFile(fileSystemFileDescriptor, fileDescriptor, &fileStat);
	log_printf("IOSUHAX_FSA_StatFile: %i", returnValue);

	if (returnValue < 0) {
		goto IOSUHAX_READ_FILE_FAILED_CLOSE;
	}

	void *fileBuffer = MEMBucket_alloc(fileStat.size, 4);
	log_printf("File Buffer: %p", fileBuffer);

	if (!fileBuffer) {
		goto IOSUHAX_READ_FILE_FAILED_CLOSE;
	}

	size_t totalBytesRead = 0;
	while (totalBytesRead < fileStat.size) {
		size_t remainingBytes = fileStat.size - totalBytesRead;
		int bytesRead = IOSUHAX_FSA_ReadFile(fileSystemFileDescriptor,
											 fileBuffer + totalBytesRead,
											 0x01,
											 remainingBytes,
											 fileDescriptor,
											 0);
		log_printf("IOSUHAX_FSA_ReadFile: %i", bytesRead);

		if (bytesRead <= 0) {
			goto IOSUHAX_READ_FILE_FAILED_CLOSE;
		} else {
			totalBytesRead += bytesRead;
		}
	}

	log_printf("Bytes read: %i", totalBytesRead);

	IOSUHAX_READ_FILE_FAILED_CLOSE:

	returnValue = IOSUHAX_FSA_CloseFile(fileSystemFileDescriptor, fileDescriptor);
	log_printf("IOSUHAX_FSA_CloseFile: %i", returnValue);

	IOSUHAX_OPEN_FILE_FAILED:

	returnValue = IOSUHAX_FSA_Close(fileSystemFileDescriptor);
	log_printf("IOSUHAX_FSA_Close: %i", returnValue);

	IOSUHAX_FSA_OPEN_FAILED:

	returnValue = IOSUHAX_Close();
	log_printf("IOSUHAX_Close: %i", returnValue);

	IOSUHAX_OPEN_FAILED:*/

};

void CommandHandler::command_get_version_hash(){
	((int *)buffer)[0] = VERSION_HASH;
	ret = sendwait(4);
};

void CommandHandler::command_persist_assembly(){
	unsigned int length = receiveString((unsigned char *)buffer, DATA_BUFFER_SIZE);
	persistAssembly(buffer, length);
};

void CommandHandler::command_clear_assembly(){
	clearAssembly();
};