#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"

typedef void onLoadFunction(bool, GameMemory*);
static void onLoadStub(bool, GameMemory*) {}

typedef void updateFunction(GameMemory*, PlayerInput*, float, float);
static void updateStub(GameMemory*, PlayerInput*, float, float) {}

typedef void renderFunction(GameMemory*, PlayerInput*, float);
static void renderStub(GameMemory*, PlayerInput*, float) {}

struct GameAPI {
    onLoadFunction* onLoad;
    updateFunction* update;
    renderFunction* render;
};

#endif