#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"

typedef void startFunction(GameMemory*);
static void startStub(GameMemory*) {}
typedef void updateFunction(GameMemory*);
static void updateStub(GameMemory*) {}
typedef void renderFunction(GameMemory*);
static void renderStub(GameMemory*) {}

struct GameAPI {
    startFunction* start;
    updateFunction* update;
    renderFunction* render;
};

#endif