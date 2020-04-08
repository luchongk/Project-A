#include <iostream>

#include "common.h"
#include "reflection.h"

#include "stb_image.cpp"
#include "linear_allocator.cpp"
#include "shader_manager.cpp"
#include "shader.cpp"

#define DLLEXPORT extern "C" __declspec(dllexport)

//static unsigned int VAO[2];
//static unsigned int VBO[2];

static float vertices[]{
    -1.0f,
    -0.5f,
    0.0f,
    0.0f,
    0.0f,
    -0.5f,
    -0.5f,
    0.0f,
    0.5f,
    1.0f,
    -0.75f,
    0.5f,
    0.0f,
    1.0f,
    0.0f,
};

static float vertices2[]{
    -0.5f,
    -0.5f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    -0.5f,
    0.0f,
    0.5f,
    1.0f,
    -0.25f,
    0.5f,
    0.0f,
    1.0f,
    0.0f,
};

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
    LinearAllocator allocator;
    ShaderManager shaderManager;
    float vertices[15];
    float vertices2[15];
    float ble;
    unsigned int VAO[2];
    unsigned int VBO[2];
    unsigned int texture;
    uint8_t storage[megabytes(4)];

    GameState()
        : allocator{megabytes(4), storage},
          shaderManager{&allocator}
    {
    }

    REFLECT()
};

REFLECTION_REGISTRATION(GameState)
{
    CLASS->addField("allocator", &GameState::allocator)
        ->addField("shaderManager", &GameState::shaderManager)
        ->addField("vertices", &GameState::vertices)
        ->addField("vertices2", &GameState::vertices2)
        ->addField("ble", &GameState::ble)
        ->addField("VAO", &GameState::VAO)
        ->addField("VBO", &GameState::VBO)
        ->addField("texture", &GameState::texture)
        ->addField("storage", &GameState::storage);
}

struct GameData
{
    Introspection meta;
    GameState state;

    /*GameData() : meta{(uint8_t*)this + sizeof(GameData)},
                 state{(uint8_t*)this + sizeof(GameData) + meta.alloc.getSize()}
    {}*/
};

static void mutateState(void *oldState, void *newState, reflection::Type *oldType, reflection::Type *newType)
{
    if (newType->fieldCount == 0)
    {
        memcpy(newState, oldState, newType->size);
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
            mutateState((uint8_t *)oldState + field->offset,
                        (uint8_t *)newState + newFields[i].offset,
                        field->type,
                        newFields[i].type);
            continue;
        }

        newFields[i].type->constructor((uint8_t *)newState + newFields[i].offset);
    }
}

DLLEXPORT void onLoad(GameMemory *gameMemory)
{
    gladLoadGL();

    //TODO: I might be able to hide the swap in the platform layer
    int prevDataIndex = gameMemory->currentDataIndex;
    if (gameMemory->isInitialized) {
        gameMemory->currentDataIndex = 1 - prevDataIndex;    //Swap memory blocks
    }

    GameData *data = new (gameMemory->data[gameMemory->currentDataIndex]) GameData{};
    reflection::Type *newType = data->meta.types.create<GameState>();

    if (!gameMemory->isInitialized)
    {
        glGenTextures(1, &data->state.texture);
        glBindTexture(GL_TEXTURE_2D, data->state.texture);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

        //Create VAO
        glGenVertexArrays(2, data->state.VAO);
        glGenBuffers(2, data->state.VBO);

        //TRIANGLE 1
        glBindVertexArray(data->state.VAO[0]);
        glBindBuffer(GL_ARRAY_BUFFER, data->state.VBO[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        //TRIANGLE 2
        glBindVertexArray(data->state.VAO[1]);
        glBindBuffer(GL_ARRAY_BUFFER, data->state.VBO[1]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        //std::cout << sizeof(gameState->vertices) << std::endl;

        for (int i = 0; i < 15; i++)
        {
            data->state.vertices[i] = vertices[i];
            data->state.vertices2[i] = vertices2[i];
        }

        gameMemory->isInitialized = true;
    }
    else
    {
        GameData *prevData = (GameData *)gameMemory->data[prevDataIndex];
        reflection::Type *oldType = prevData->meta.types.get<GameState>();
        mutateState(&prevData->state, &data->state, oldType, newType);
        prevData->meta.allocator.clear();

        data->state.allocator.clear();
    }

    char *vertexShaderSrc =
        "#version 460 core\n"
        "layout (location = 0) in vec3 inPosition;\n"
        "layout (location = 1) in vec2 inTexCoord;\n"
        "out vec3 Position;\n"
        "out vec2 TexCoord;\n\n"
        "void main() {\n"
        "    Position = inPosition;\n"
        "    TexCoord = inTexCoord;\n"
        "    gl_Position = vec4(inPosition.x + 0.001, inPosition.y, inPosition.z, 1.0f);\n"
        "}";

    char *fragmentShaderSrc =
        "#version 460 core\n"
        "in vec3 Position;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform float Color;\n"
        "uniform sampler2D s;\n"
        "void main() {\n"
        "    FragColor = texture(s, TexCoord) * vec4(Color, 1 - Color, Color, 1.0f);\n"
        "}";


    data->state.shaderManager.setShader("bla", vertexShaderSrc, fragmentShaderSrc);
}

DLLEXPORT void update(GameMemory *gameMemory)
{
    GameData *data = (GameData *)gameMemory->data[gameMemory->currentDataIndex];

    for (int i = 0; i < 15; i++)
    {
        if (i % 5 == 0)
            data->state.vertices[i] += 0.0001f;
    }

    for (int i = 0; i < 15; i++)
    {
        if (i % 5 == 0)
            data->state.vertices2[i] += 0.0f;
    }
}

DLLEXPORT void render(GameMemory *gameMemory)
{
    GameData *data = (GameData *)gameMemory->data[gameMemory->currentDataIndex];

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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

    Shader *bla = data->state.shaderManager.getShader("bla");
    bla->use();
    bla->setFloat("Color", 0.2f);

    glBindTexture(GL_TEXTURE_2D, data->state.texture);
    glBindVertexArray(data->state.VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, data->state.VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data->state.vertices), data->state.vertices, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 5);

    bla->setFloat("Color", 0.7f);
    glBindVertexArray(data->state.VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, data->state.VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data->state.vertices2), data->state.vertices2, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 5);
}