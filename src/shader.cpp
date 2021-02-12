#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "glad/glad.h"
#include "shader.h"

static void compile_shader(Shader* shader, const char* name, const char* vertex_path, const char* fragment_path) {
    int i = 0;
    while(*name) {
        shader->name[i++] = *name++;
    }
    shader->name[i] = '\0';

    char* vertex_src = os_read_entire_file(vertex_path);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_src, nullptr);
    glCompileShader(vertexShader);

    int success = 0;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;     //TODO: LOG THIS
        return;
    }

    char* fragment_src = os_read_entire_file(fragment_path);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_src, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;   //TODO: LOG THIS
        return;
    }

    shader->ID = glCreateProgram();
    glAttachShader(shader->ID, vertexShader);
    glAttachShader(shader->ID, fragmentShader);
    glLinkProgram(shader->ID);

    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shader->ID, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;   //TODO: LOG THIS
        return;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

static void delete_shader(Shader* shader) {
    glDeleteProgram(shader->ID);
}

static void use_shader(Shader* shader) {
    glUseProgram(shader->ID);
}

static void set_shader_uniform(Shader* shader, const char* name, bool value) {
    glUniform1ui(glGetUniformLocation(shader->ID, name), value);
}

static void set_shader_uniform(Shader* shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->ID, name), value);
}

static void set_shader_uniform(Shader* shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader->ID, name), value);
}

static void set_shader_uniform(Shader* shader, const char* name, const glm::vec3 &vec)
{
    glUniform3fv(glGetUniformLocation(shader->ID, name), 1, &vec[0]);
}

static void set_shader_uniform(Shader* shader, const char* name, const glm::mat4 &mat)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, &mat[0][0]);
}