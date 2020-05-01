#include <iostream>
#include <iomanip>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "common.h"
#include "reflection.h"

#include "stb_image.cpp"
#include "linear_allocator.cpp"
#include "shader_manager.cpp"
#include "shader.cpp"

#define DLLEXPORT extern "C" __declspec(dllexport)

struct Introspection
{
    LinearAllocator allocator;
    reflection::TypeDB types;
    uint8_t allocMemory[kilobytes(64)];

    Introspection()
        : allocator{kilobytes(64), allocMemory},
          types{&allocator}
    {
    }
};

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
    glm::vec3 prevCameraPos;
    float prevCameraYaw;
    float prevCameraPitch;
    float moveDir1;
    float cubesRotation;
    float prevCubesRotation;
    bool paused = false;
    uint8_t reloadablesMemory[megabytes(4)];

    GameState()
        : reloadablesAlloc{megabytes(4), reloadablesMemory},
          shaderManager{&reloadablesAlloc},
          moveDir1{1},
          cameraPos{glm::vec3{-1,0,0}},
          cameraForward{glm::vec3{0,0,-1}},
          cameraYaw{-90.0f},
          cameraPitch{0.0f}
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
        ->addField("rotation", &GameState::cubesRotation)
        ->addField("paused", &GameState::paused)
        ->addField("reloadablesMemory", &GameState::reloadablesMemory);
}

struct GameData
{
    Introspection meta;
    GameState state;

    void mutateState(void *oldState, reflection::Type *oldType, reflection::Type *newType)
    {
        mutateObject(oldState, &state, oldType, newType);
    }
private:
    void mutateObject(void *oldObject, void *newObject, reflection::Type *oldType, reflection::Type *newType) {
        if (newType->fieldCount == 0)
        {
            memcpy(newObject, oldObject, newType->size);
            return;
        }

        reflection::Field *newFields = newType->fields;
        for (int i = 0; i < newType->fieldCount; i++)
        {
            if(newFields[i].isPointer) {
                continue;
            }

            reflection::Field *field = oldType->findField(newFields[i].name);
            if (field)
            {
                mutateObject((uint8_t *)oldObject + field->offset,
                            (uint8_t *)newObject + newFields[i].offset,
                            field->type,
                            newFields[i].type);
                continue;
            }

            newFields[i].type->constructor((uint8_t *)newObject + newFields[i].offset);
        }
    }
};

DLLEXPORT void onLoad(GameMemory *gameMemory)
{
    gladLoadGL();

    //TODO: I might be able to hide the swap in the platform layer?
    int prevDataIndex = gameMemory->currentDataIndex;
    if (gameMemory->isInitialized) {
        gameMemory->currentDataIndex = 1 - prevDataIndex;    //Swap memory blocks
    }

    //! This constructor initializes everything, even fields that will be replaced during mutation.
    //TODO: Find a way to only initialize new fields with default values, leaving old fields intact.
    GameData *data = new (gameMemory->data[gameMemory->currentDataIndex]) GameData{};
    reflection::Type *newType = data->meta.types.create<GameState>();

    if (!gameMemory->isInitialized)
    {
        glEnable(GL_DEPTH_TEST);

        float cubeModel[] {
            -0.5f, -0.5f, -0.5f,  0.4f, 0.4f,
            0.5f, -0.5f, -0.5f,  0.6f, 0.4f,
            0.5f,  0.5f, -0.5f,  0.6f, 0.6f,
            0.5f,  0.5f, -0.5f,  0.6f, 0.6f,
            -0.5f,  0.5f, -0.5f,  0.4f, 0.6f,
            -0.5f, -0.5f, -0.5f,  0.4f, 0.4f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };

        //Create VAO
        glGenVertexArrays(1, &data->state.VAO);
        glGenBuffers(1, &data->state.VBO);

        glBindVertexArray(data->state.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, data->state.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeModel), cubeModel, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glGenTextures(1, &data->state.texture);
        glBindTexture(GL_TEXTURE_2D, data->state.texture);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load and generate the texture
        int width, height, nrChannels;
        unsigned char *image_data = stbi_load("..\\Untitled.png", &width, &height, &nrChannels, 0);
        if (image_data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            printf("Failed to load texture\n");
        }
        stbi_image_free(image_data);

        gameMemory->isInitialized = true;
    }
    else
    {
        GameData *prevData = (GameData *)gameMemory->data[prevDataIndex];
        reflection::Type *oldType = prevData->meta.types.get<GameState>();
        data->mutateState(&prevData->state, oldType, newType);
        prevData->meta.allocator.clear();

        data->state.reloadablesAlloc.clear();
    }

    char *vertexShaderSrc =
        "#version 460 core\n"
        "layout (location = 0) in vec3 inPosition;\n"
        "layout (location = 1) in vec2 inTexCoord;\n"
        "out vec3 Position;\n"
        "out vec2 TexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n\n"

        "void main() {\n"
        "    Position = inPosition;\n"
        "    TexCoord = inTexCoord;\n"
        "    gl_Position = projection * view * model * vec4(inPosition, 1.0);\n"
        "}";

    char *fragmentShaderSrc =
        "#version 460 core\n"
        "in vec3 Position;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform float Color;\n"
        "uniform sampler2D s;\n\n"

        "void main() {\n"
        "    FragColor = texture(s, TexCoord) * vec4(Color, 1 - Color, Color, 1.0f);\n"
        "}";


    Shader *bla = data->state.shaderManager.setShader("bla", vertexShaderSrc, fragmentShaderSrc);
    bla->use();
}

DLLEXPORT void update(GameMemory *gameMemory, PlayerInput* input, float dt, float time)
{
    GameState *state = (GameState*)&((GameData *)gameMemory->data[gameMemory->currentDataIndex])->state;

    if(input->pause) {
        state->paused = !state->paused;
    }

    if(state->paused) {
        dt = 0;
    }

    state->prevCameraPitch = state->cameraPitch;
    state->prevCameraYaw = state->cameraYaw;
    state->prevCameraPos = state->cameraPos;
    state->prevCubesRotation = state->cubesRotation;

    state->cameraYaw += input->mouseDeltaX * 2.0f * dt;
    /*if(state->cameraYaw > 360) {
        state->cameraYaw -= 360;
    }
    else if(state->cameraYaw < 0) {
        state->cameraYaw += 360;
    }*/
    state->cameraPitch += -input->mouseDeltaY * 2.0f * dt;
    /*if(state->cameraPitch > 360) {
        state->cameraPitch -= 360;
    }
    else if(state->cameraPitch < 0) {
        state->cameraPitch += 360;
    }*/
    state->cameraForward.x = glm::cos(glm::radians(state->cameraYaw)) * glm::cos(glm::radians(state->cameraPitch));
    state->cameraForward.y = glm::sin(glm::radians(state->cameraPitch));
    state->cameraForward.z = glm::sin(glm::radians(state->cameraYaw)) * glm::cos(glm::radians(state->cameraPitch));
    state->cameraForward = glm::normalize(state->cameraForward);

    if(input->horizontal != 0) {
        state->cameraPos += glm::cross(state->cameraForward, glm::vec3{0,1,0}) * 3.0f * (float)input->horizontal * dt;;
    }

    if(input->vertical != 0)
        state->cameraPos += state->cameraForward * 3.0f * (float)input->vertical * dt;
    
    /*if(input->horizontal * state->moveDir1 < 0)
        state->moveDir1 += 10 * (state->moveDir1 >= 0 ? -dt : dt);
    else if(input->horizontal * state->moveDir1 > 0 || input->horizontal != 0 && state->moveDir1 == 0)
        state->moveDir1 -= 2 * (state->moveDir1 >= 0 ? -dt : dt);*/
    
    state->cubesRotation += 0.0f * state->moveDir1 * dt;
    //std::cout << "time: " << time << std::endl;
}

DLLEXPORT void render(GameMemory *gameMemory, PlayerInput* input, float deltaInterpolation)
{
    GameState *state = (GameState*)&((GameData *)gameMemory->data[gameMemory->currentDataIndex])->state;

#if 0
    std::cout << "checking for shader programs:\n";
    for(int i = 0; i < 50; i++) {
        if(glIsProgram(i)) {
            int status;
            glGetProgramiv(i, GL_DELETE_STATUS, &status);
            if(!status) {
                std::cout << i << " is program! ";
            }
        }
    }
    std::cout << std::endl;
#endif
    float cameraYaw = state->cameraYaw * deltaInterpolation + state->prevCameraYaw * (1 - deltaInterpolation);
    float cameraPitch = state->cameraPitch * deltaInterpolation + state->prevCameraPitch * (1 - deltaInterpolation);
    glm::vec3 cameraForward;
    cameraForward.x = glm::cos(glm::radians(cameraYaw)) * glm::cos(glm::radians(cameraPitch));
    cameraForward.y = glm::sin(glm::radians(cameraPitch));
    cameraForward.z = glm::sin(glm::radians(cameraYaw)) * glm::cos(glm::radians(cameraPitch));
    cameraForward = glm::normalize(cameraForward);
    
    glm::vec3 cameraPos = state->cameraPos * deltaInterpolation + state->prevCameraPos * (1 - deltaInterpolation);
    float cubesRotation = state->cubesRotation * deltaInterpolation + state->prevCubesRotation * (1 - deltaInterpolation);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    Shader* bla = state->shaderManager.getShader("bla");
    bla->use();
    bla->setMat4("view", glm::lookAt(cameraPos, cameraPos + cameraForward, glm::vec3{0,1,0}));
    bla->setMat4("projection", projection);

    float color1 = std::fabsf(std::sinf(3.1415f * (cubesRotation * 0.5f + 0.5f)));

    bla->setFloat("Color", color1);

    glm::vec3 cubePositions[] {
        glm::vec3{0.0f, 0.0f, 0.0f},
        glm::vec3{2.0f, 5.0f, -15.0f},
        glm::vec3{-1.5f, -2.2f, -2.5f},
        glm::vec3{-3.8f, -2.0f, -12.3f},
        glm::vec3{2.4f, -0.4f, -3.5f},
    };

    glBindVertexArray(state->VAO);
    for(int i = 0; i < 5; i++) {
        glm::mat4 localToWorld = glm::mat4{1.0f};
        localToWorld = glm::translate(localToWorld, cubePositions[i]);
        float angle = 50.0f * (i+1);
        localToWorld = glm::rotate(localToWorld, cubesRotation * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
        bla->setMat4("model", localToWorld);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}