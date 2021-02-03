#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "vector.h"
#include "memory.h"
#include "shader.h"

struct Camera {
    Vector3 position;
    Vector3 forward;
    float yaw = -90;
    float pitch;
};

struct GlobalState {
    LinearArena temporary_memory;
    
    Shader light_shader;
    Shader shader;
    unsigned int lightVAO;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;
    Camera camera;
    Vector3 light_pos = {1.0f, 1.0f, -6.0f};
    float cubesRotationDir = 1;
    float cubesRotation;
    bool paused = false;
};

GlobalState* game_state;
PlatformAPI* platform;

#endif