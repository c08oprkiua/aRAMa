#ifndef ARAMA_NOTIFICATION_AND_LOG_H
#define ARAMA_NOTIFICATION_AND_LOG_H

#include <coreinit/debug.h>
//TODO: remove the need for this by switching __os__snprintf to use
//the log/notification class
#include <coreinit/internal.h>

#define CHECK_ERROR(cond)     \
	if (cond)                 \
	{                         \
		line = __LINE__; \
		goto error;       \
}

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

#define ASSERT_MINIMUM_HOLDS(actual, minimum, variableName) \
if(actual < minimum) { \
    char buffer[100] = {0}; \
    __os_snprintf(buffer, 100, "%s: Limit exceeded (minimum: %i, actual: %i)", variableName, minimum, actual); \
    OSFatal(buffer); \
} \

#define ASSERT_MAXIMUM_HOLDS(maximum, actual, variableName) \
if(actual > maximum) { \
    char buffer[100] = {0}; \
    __os_snprintf(buffer, 100, "%s: Limit exceeded (maximum: %i, actual: %i)", variableName, maximum, actual); \
    OSFatal(buffer); \
} \

#define ASSERT_FUNCTION_SUCCEEDED(returnValue, functionName) \
    if (returnValue < 0) { \
        char buffer[100] = {0}; \
        __os_snprintf(buffer, 100, "%s failed with return value: %i", functionName, returnValue); \
        OSFatal(buffer); \
    } \

#define ASSERT_VALID_EFFECTIVE_ADDRESS(effectiveAddress, message) \
    if(!OSIsAddressValid((void *) effectiveAddress)) { \
    char buffer[100] = {0}; \
        __os_snprintf(buffer, 100, "Address %04x invalid: %s", effectiveAddress, message); \
        OSFatal(buffer); \
    }

#define ASSERT_INTEGER(actual, expected, name) \
    if(actual != expected) { \
        char buffer[50] = {0}; \
        __os_snprintf(buffer, 50, "%s assertion failed: %i == %i", name, actual, expected); \
        OSFatal(buffer); \
    }

#define ASSERT_STRING(actual, expected) \
    if(strcmp(actual, expected) != 0) { \
        char buffer[50] = {0}; \
        __os_snprintf(buffer, 50, "String assertion failed: \"%s\" == \"%s\"", actual, expected); \
        OSFatal(buffer); \
    }

#define ASSERT_ALLOCATED(variable, name) \
    if(variable == 0) { \
        char buffer[50] = {0}; \
        __os_snprintf(buffer, 50, "%s allocation failed", name); \
        OSFatal(buffer); \
    }

#endif