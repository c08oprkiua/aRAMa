#include "../command_handler.h"

#include <whb/log.h>
#include <coreinit/mcp.h>
#include <nn/act.h>

#define VERSION_HASH 0x7FB223
#define SERVER_VERSION "07/1/2024"

void CommandHandler::command_server_status(){
	ret = sendByte(1);
	CHECK_ERROR(ret < 0)

	error:
	error = ret;
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

void CommandHandler::command_account_identifier(){
	// Acquire the RPL

	/*Unneeded in WUT

	uint32_t nn_act_handle;
	OSDynLoad_Acquire("nn_act.rpl", &nn_act_handle);

	// Acquire the functions via their mangled file names
	int (*nn_act_Initialize)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "Initialize__Q2_2nn3actFv", &nn_act_Initialize);
	ASSERT_ALLOCATED(nn_act_Initialize, "nn_act_Initialize")
	uint8_t (*nn_act_GetSlotNo)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetSlotNo__Q2_2nn3actFv", &nn_act_GetSlotNo);
	ASSERT_ALLOCATED(nn_act_GetSlotNo, "nn_act_GetSlotNo")
	uint32_t (*nn_act_GetPersistentIdEx)(uint8_t);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetPersistentIdEx__Q2_2nn3actFUc", &nn_act_GetPersistentIdEx);
	ASSERT_ALLOCATED(nn_act_GetPersistentIdEx, "nn_act_GetPersistentIdEx")
	int (*nn_act_Finalize)(void);
	OSDynLoad_FindExport(nn_act_handle, 0, "Finalize__Q2_2nn3actFv", &nn_act_Finalize);
	ASSERT_ALLOCATED(nn_act_Finalize, "nn_act_Finalize")
	*/

	// Get the identifier
	ret = (int) nn::act::Initialize();
	// ASSERT_INTEGER(ret, 1, "Initializing account library");

	nn::act::SlotNo slotNumber = nn::act::GetSlotNo();
	nn::act::PersistentId persistentIdentifier = nn::act::GetPersistentIdEx(slotNumber);
	ret = (int) nn::act::Finalize();
	ASSERT_FUNCTION_SUCCEEDED(ret, "nn_act_Finalize");

	// Send it
	ret = sendwait_buffer((uint8_t *)&persistentIdentifier, 4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (persistent identifier)");
};

void CommandHandler::command_get_os_version(){
	MCPSystemVersion version;
	//Idk what to put for handle
	MCP_GetSystemVersion(0, &version);

	((int *)buffer)[0] = version.major;
	((int *)buffer)[1] = version.minor;
	((int *)buffer)[2] = version.patch;
	buffer[3] = version.region;
	ret = sendwait(3);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (OS version)");
};

void CommandHandler::command_get_data_buffer_size(){
	WHBLogPrintf("COMMAND_GET_DATA_BUFFER_SIZE...\n");
	((int *)buffer)[0] = DATA_BUFFER_SIZE;
	WHBLogPrintf("Sending buffer size...\n");
	ret = sendwait(sizeof(int));
	WHBLogPrintf("Sent: %i\n", ret);
	CHECK_ERROR(ret < 0)

	error:
	error = ret;
	return;
};

void CommandHandler::command_get_version_hash(){
	((int *)buffer)[0] = VERSION_HASH;
	ret = sendwait(4);
};

#define CODE_HANDLER_INSTALL_ADDRESS 0x010F4000
//Todo: send null back if code handler is not initialized
void CommandHandler::command_get_code_handler_address(){
	((int *)buffer)[0] = CODE_HANDLER_INSTALL_ADDRESS;
	ret = sendwait(4);
	ASSERT_FUNCTION_SUCCEEDED(ret, "sendwait (code handler address)")
}