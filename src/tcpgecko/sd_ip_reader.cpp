#include "sd_ip_reader.h"

#include <whb/log.h>

char ipFromSd[16];
bool hasReadIP = false;

void initializeUDPLog() {
	if (!hasReadIP) {
		WHBLogPrintf("Reading ip from sd card\n");
		hasReadIP = true;

		std::string ipFilePath = std::string(SD_PATH) + WIIU_PATH + "/" + IP_TXT;

		CFile file(ipFilePath, CFile::ReadOnly);
		if (!file.isOpen()) {
			WHBLogPrintf("File %s not found, using hard-coded\n", ipFilePath.c_str());
			log_init(COMPUTER_IP_ADDRESS);
			return;
		}

		std::string strBuffer;
		strBuffer.resize(file.size());
		file.read((uint8_t *) &strBuffer[0], strBuffer.size());

		if (strBuffer.length() >= sizeof(ipFromSd)) {
			WHBLogPrintf("Loading ip from sd failed. String was too long: %s\n", strBuffer.c_str());
			return;
		}

		memcpy(ipFromSd, strBuffer.c_str(), strBuffer.length());
		ipFromSd[strBuffer.length()] = 0;

		WHBLogPrintf("Successfully read ip from sd! ip is: %s\n", ipFromSd);

		log_init(ipFromSd);
	}

	if (strlen(ipFromSd) > 0) {
		log_init(ipFromSd);
	}
}