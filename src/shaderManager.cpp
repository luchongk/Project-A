#include <new>
#include <string.h>

#include "shader.cpp"

unsigned long ShaderManager::hash(char* str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

ShaderManager::ShaderManager(size_t sizeIn) {
    this->shaders = (Shader*)(this + 1);
    this->size = sizeIn * sizeof(Shader);
}

void ShaderManager::setShader(char* name, char* vertexSrc, char* fragmentSrc) {
    size_t pos = this->hash(name);
    const size_t length = this->size / sizeof(Shader);
    pos = pos % length;

    for(int i = 0; i < length; i++) {
        if(shaders[pos].getId() == 0 || !strcmp(name, shaders[pos].getName())) {
            new (shaders + pos) Shader{name, vertexSrc, fragmentSrc};
            return;
        }
        pos = (pos + 1) % length;
    }
    
    assert(!"Out of memory for shaders!");
}

size_t ShaderManager::getSize() { return this->size; }

Shader* ShaderManager::getShader(char* name) {
    size_t pos = this->hash(name);
    const size_t length = this->size / sizeof(Shader);
    pos = pos % length;

    for(int i = 0; i < length; i++) {
        if(shaders[pos].getId() == 0)
            return nullptr;
        else if(!strcmp(name, shaders[pos].getName()))
            return shaders + pos;
        pos = (pos + 1) % length;
    }
    
    return nullptr;
};