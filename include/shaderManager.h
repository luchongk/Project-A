#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "shader.h"

class ShaderManager {
    size_t size;
    Shader* shaders;

    unsigned long hash(char* str);

public:
    ShaderManager(size_t size);
    void setShader(char* name, char* vertexSrc, char* fragmentSrc);
    size_t getSize();
    Shader* getShader(char* name);
};

#endif