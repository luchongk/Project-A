#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include "glad/glad.h"

class ShaderManager;

struct GameMemory {
    size_t totalSize;
    ShaderManager* shaderManager;
    void* data;
};

#endif