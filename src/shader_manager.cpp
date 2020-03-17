#include <string.h>

#include "utils.h"
#include "shader_manager.h"
#include "linear_allocator.h"

unsigned long ShaderManager::hash(char* str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

ShaderManager::ShaderManager(size_t count, LinearAllocator* allocator) : allocator(allocator) {
    this->shaders = (Shader**)(this + 1);
    this->size = count;
}

void ShaderManager::setShader(char* name, char* vertexSrc, char* fragmentSrc) {
    size_t pos = this->hash(name);
    const size_t length = this->size;
    pos = pos % length;

    for(int i = 0; i < length; i++) {
        if(shaders[pos] == nullptr) {
            shaders[pos] = allocator->push<Shader>(name, vertexSrc, fragmentSrc);
            return;
        }
        
        assert(!strcmp(name, shaders[pos]->getName()) && !"Shader already exists");

        pos = (pos + 1) % length;
    }
    
    assert(!"Out of memory for shaders!");
}

size_t ShaderManager::getSize() { return this->size; }

Shader* ShaderManager::getShader(char* name) {
    size_t pos = this->hash(name);
    const size_t length = this->size;
    pos = pos % length;

    for(int i = 0; i < length; i++) {
        if(shaders[pos] == nullptr)
            continue;
        else if(!strcmp(name, shaders[pos]->getName()))
            return shaders[pos];
        pos = (pos + 1) % length;
    }
    
    return nullptr;
};