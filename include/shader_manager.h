#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

class Shader;
class LinearAllocator;

class ShaderManager {
    size_t size;
    Shader** shaders;
    LinearAllocator* allocator;

    unsigned long hash(char* str);

public:
    ShaderManager(size_t size, LinearAllocator* allocator);
    void setShader(char* name, char* vertexSrc, char* fragmentSrc);
    size_t getSize();
    Shader* getShader(char* name);
};

#endif