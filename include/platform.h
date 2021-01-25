#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"

typedef void onLoadFunction(bool, GameMemory*, PlatformAPI*);
static void onLoadStub(bool, GameMemory*, PlatformAPI*) {}

typedef void updateFunction(GameMemory*, PlayerInput*, float, float);
static void updateStub(GameMemory*, PlayerInput*, float, float) {}

typedef void renderFunction(GameMemory*, float);
static void renderStub(GameMemory*, float) {}

struct GameAPI {
    onLoadFunction* onLoad;
    updateFunction* update;
    renderFunction* render;
};

#endif