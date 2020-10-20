#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "reflection.h"
#include "string_map.h"
#include "shader.h"

class LinearAllocator;

class ShaderManager {
    REFLECT
    
    LinearAllocator* allocator;
    StringMap<Shader> shaderMap;

public:
    ShaderManager() = default;
    ShaderManager(LinearAllocator* allocator);
    Shader* setShader(char* name, char* vertexSrc, char* fragmentSrc);
    Shader* getShader(char* name);
    void clear();
};

/* REFLECTION_REGISTRATION(ShaderManager) {
    CLASS->addField("allocator", &ShaderManager::allocator)
        ->addField("shaderMap", &ShaderManager::shaderMap);
} */

#endif