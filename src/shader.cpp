#include <iostream>

#include "shader.h"

Shader::Shader(const char* name, const char* vertexSrc, const char* fragmentSrc) {
    int i = 0;
    while(*name) {
        this->name[i++] = *name++;
    }
    this->name[i] = '\0';

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    int success = 0;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;     //TODO: LOG THIS
        return;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;   //TODO: LOG THIS
        return;
    }

    this->ID = glCreateProgram();
    glAttachShader(this->ID, vertexShader);
    glAttachShader(this->ID, fragmentShader);
    glLinkProgram(this->ID);

    glGetProgramiv(this->ID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(this->ID, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;   //TODO: LOG THIS
        return;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

const char* Shader::getName() {
    return this->name;
}

unsigned int Shader::getId() {
    return ID;
}

void Shader::use() {
    glUseProgram(this->ID);
}

void Shader::setBool(const char* name, bool value) {
    glUniform1ui(glGetUniformLocation(this->ID, name), value);
}

void Shader::setInt(const char* name, int value) {
    glUniform1i(glGetUniformLocation(this->ID, name), value);
}

void Shader::setFloat(const char* name, float value) {
    glUniform1f(glGetUniformLocation(this->ID, name), value);
}