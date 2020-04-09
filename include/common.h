#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include "glad/glad.h"

struct GameMemory {
    bool isInitialized;
    size_t totalSize;
    void* data[2];
    int currentDataIndex;
};

struct PlayerInput {
    bool quit;
    bool pause;
    int horizontal;
};

#endif