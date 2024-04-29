#include "../command_handler.h"

#include <coreinit/cache.h>

void CommandHandler::command_write_8()
{
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0);

	char *destinationAddress = ((char **)buffer)[0];
	*destinationAddress = buffer[7];
	DCFlushRange(destinationAddress, 1);

	error:
	error = ret;
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

	error:
	error = ret;
	return;
};

void CommandHandler::command_write_32()
{
	int destinationAddress, value;
	ret = recvwait(sizeof(int) * 2);
	CHECK_ERROR(ret < 0)

	destinationAddress = ((int *)buffer)[0];
	value = ((int *)buffer)[1];

	GeckoKernelCopyData((uint8_t *) destinationAddress, (uint8_t *) &value, sizeof(int));
	
	error:
	error = ret;
	return;
};