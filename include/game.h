#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "vector.h"
#include "memory.h"
#include "shader.h"

struct GlobalState {
    LinearArena temporary_memory;
    
    Shader light_shader;
    Shader shader;
    unsigned int lightVAO;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;
    Vector3 lastCameraPos;
    Vector3 cameraPos;
    Vector3 cameraForward;
    float cameraYaw = -90;
    float cameraPitch;
    float cubesRotationDir = 1;
    float cubesRotation;
    bool paused = false;
};

GlobalState* game_state;
PlatformAPI* platform;

#endif