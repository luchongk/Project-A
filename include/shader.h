#ifndef SHADER_H
#define SHADER_H

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

struct Shader {
    unsigned int ID;
    char name[64];
};

void compile_shader(Shader* shader, const char* name, const char* vertex_path, const char* fragment_path);
void delete_shader(Shader* shader);

void use_shader(Shader* shader);

void set_shader_uniform(Shader* shader, const char* name, bool value);
void set_shader_uniform(Shader* shader, const char* name, int value);
void set_shader_uniform(Shader* shader, const char* name, float value);
void set_shader_uniform(Shader* shader, const char* name, const glm::vec3 &vec);
void set_shader_uniform(Shader* shader, const char* name, const glm::mat4 &mat);

#endif