#ifndef ARAMA_H
#define ARAMA_H

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
};

static SingletonGroup singletons;

//This function is responsible for loading up all the things that should be active, 
//and nothing that shouldn't be active.
void aRAMaReInit();
//This function does the opposite of aRAMaReInit: It unloads all the things that
//shouldn't be active anymore
void aRAMaDeInit();

#endif