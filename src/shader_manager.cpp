#include <string.h>
#include <new>

#include "utils.h"
#include "shader_manager.h"
#include "shader.h"
#include "linear_allocator.h"

ShaderManager::ShaderManager(LinearAllocator* allocator)
    : allocator(allocator),
      shaderMap(allocator)
{ }

void ShaderManager::setShader(char* name, char* vertexSrc, char* fragmentSrc) {
    Shader* stored = shaderMap.get(name);
    if(stored) {
        destroy(stored, *allocator);
    }

    Shader* shader = new(*allocator) Shader(name, vertexSrc, fragmentSrc);
    shaderMap.set(name, shader);
}

Shader* ShaderManager::getShader(char* name) {
    return shaderMap.get(name);
};

void ShaderManager::clear() {
    shaderMap.clear();
}