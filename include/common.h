#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include "glad/glad.h"
#include "shaderManager.h"

#ifdef DEBUG
constexpr void assert(bool condition) { if(!(condition)) (*(int*)nullptr) = 0; }
#else
constexpr void assert(bool condition) { }
#endif

template<class T, size_t N>
constexpr size_t length(T (&)[N]) { return N; }

struct GameMemory {
    size_t totalSize;
    ShaderManager* shaderManager;
    void* data;
};

#endif