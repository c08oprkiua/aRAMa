#include "../command_handler.h"


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