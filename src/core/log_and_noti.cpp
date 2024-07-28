#include "log_and_noti.h"

#include <notifications/notifications.h>
#include <whb/log.h>

void aRAMaLogNoti::print(const char *message){
    WHBLogPrint(message);
}

void aRAMaLogNoti::print_w_notif(const char *message, uint8_t seconds){
    WHBLogPrint(message);
    NotificationModule_AddInfoNotificationEx(message, 
    (float) seconds, 
    {100,100,100,255},
    {255,255,255,255},
    nullptr,
    nullptr
    );

}