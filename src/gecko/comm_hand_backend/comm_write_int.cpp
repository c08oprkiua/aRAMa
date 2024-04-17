#include "../command_handler.h"

#include <coreinit/cache.h>

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