#ifndef ARAMA_H
#define ARAMA_H

#define ARAMA_PLUGIN_NAME "aRAMa"

#include "gecko/gecko_processor.h"
#include "code_handler.h"
#include "network/caffiine.h"
#include "network/saviine.h"
#include "core/log_and_noti.h"
#include "config.h"

struct SingletonGroup {
    GeckoProcessor *gecko = nullptr;
    CodeHandler *code_handler = nullptr;
    Saviine *saviine = nullptr;
    Caffiine *caffiine = nullptr;
    aRAMaLogNoti *log = nullptr;
    aRAMaConfig *config = nullptr;
};

static SingletonGroup singletons;

#endif