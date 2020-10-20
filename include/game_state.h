#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "iostream"
#include "reflection.h"
#include "shader_manager.h"
#include "vector.h"

struct GameState
{
    REFLECT
    
    LinearAllocator reloadablesAlloc;
    ShaderManager shaderManager;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;
    Vector3 cameraPos;
    Vector3 cameraForward;
    float cameraYaw;
    float cameraPitch;
    float cubesRotationDir;
    float cubesRotation;
    bool paused = false;

    GameState()
        : reloadablesAlloc{megabytes(4), this + sizeof(this)},
          shaderManager{&reloadablesAlloc},
          cubesRotationDir{1},
          cameraPos{-1,0,0},
          cameraForward{0,0,-1},
          cameraYaw{-90.0f},
          cameraPitch{0.0f}
    {
    }
};

#endif