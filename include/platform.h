#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"

typedef void updateFunction(GameMemory*, PlayerInput*);
static void updateStub(GameMemory*, PlayerInput*) {}
typedef void renderFunction(GameMemory*);
static void renderStub(GameMemory*) {}

struct GameAPI {
    updateFunction* update;
    renderFunction* render;
};

#endif