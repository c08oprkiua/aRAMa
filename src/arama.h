#ifndef ARAMA_H
#define ARAMA_H

#include "gecko/gecko_processor.h"
#include "code_handler.h"
#include "config.h"
#include "network/caffiine.h"
#include "network/saviine.h"



static GeckoProcessor *gecko = nullptr;
static CodeHandler *c_h = nullptr;
static Saviine *saviine = nullptr;
static Caffiine *caffiine = nullptr;

//This function is responsible for loading up all the things that should be active, 
//and nothing that shouldn't be active.
void aRAMaReInit();
//This function does the opposite of aRAMaReInit: It unloads all the things that
//shouldn't be active anymore
void aRAMaDeInit();


//static int runGeckoServer(int argc, const char **argv);
static int runCodeHandlerServer(int argc, const char **argv);
static int runSaviineServer(int argc, const char **argv);
static int runCaffiineServer(int argc, const char **argv);

#endif