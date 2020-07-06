#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "glm/glm.hpp"
#include "linear_allocator.h"
#include "shader_manager.h"

struct GameState
{
    LinearAllocator reloadablesAlloc;
    ShaderManager shaderManager;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;
    glm::vec3 cameraPos;
    glm::vec3 cameraForward;
    float cameraYaw;
    float cameraPitch;
    float moveDir1;
    float cubesRotation;
    bool paused = false;
    float pos;

    GameState()
        : reloadablesAlloc{megabytes(4), this + sizeof(this)},
          shaderManager{&reloadablesAlloc},
          moveDir1{1},
          cameraPos{glm::vec3{-1,0,0}},
          cameraForward{glm::vec3{0,0,-1}},
          cameraYaw{-90.0f},
          cameraPitch{0.0f},
          pos{-5.0f}
    {
    }

    REFLECT()
};

REFLECTION_REGISTRATION(GameState)
{
    CLASS->addField("reloadablesAlloc", &GameState::reloadablesAlloc)
        ->addField("shaderManager", &GameState::shaderManager)
        ->addField("VAO", &GameState::VAO)
        ->addField("VBO", &GameState::VBO)
        ->addField("texture", &GameState::texture)
        ->addField("cameraPos", &GameState::cameraPos)
        ->addField("cameraForward", &GameState::cameraForward)
        ->addField("cameraYaw", &GameState::cameraYaw)
        ->addField("cameraPitch", &GameState::cameraPitch)
        ->addField("moveDir1", &GameState::moveDir1)
        ->addField("cubesRotation", &GameState::cubesRotation)
        ->addField("paused", &GameState::paused)
        ->addField("pos", &GameState::pos);
}

#endif