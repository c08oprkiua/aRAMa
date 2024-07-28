#ifndef ARAMA_NOTIFICATION_AND_LOG_H
#define ARAMA_NOTIFICATION_AND_LOG_H

#include <coreinit/debug.h>

#define CHECK_ERROR(cond)     \
	if (cond)                 \
	{                         \
		line = __LINE__; \
		goto error;       \
}

#define ASSERT_FUNCTION_SUCCEEDED(returnValue, functionName) \
    if (returnValue < 0) { \
        char buffer[100] = {0}; \
        WHBLogPrintf(buffer, 100, "%s failed with return value: %i", functionName, returnValue); \
        OSFatal(buffer); \
} \

class aRAMaLogNoti {
private:
    bool notis_enabled; //extra on top of the regular setting in case init fails

public:

    void print(const char *message);
    void print_w_notif(const char *message, uint8_t seconds);
    void notification(const char *message, uint8_t seconds);

    void printf();
    void printf_w_notif();
    void notificationf();

};

#endif