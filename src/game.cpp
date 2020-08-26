#include "iostream"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "common.h"
#include "linear_allocator.h"
#include "shader_manager.h"

#include "shader.cpp"
#include "shader_manager.cpp"
#include "stb_image.cpp"

class A {
public:
    int ala;

    virtual int some() {
        return 1;
    }
    
    virtual void say() {
        std::cout << "I say A" << std::endl;
    }

    REFLECT()
};

REFLECTION_REGISTRATION(A)
{
    CLASS->addField("ala", &A::ala);
}

class B : public A {
public:
    int bla;

    virtual int some() {
        return 4;
    }

    virtual void say() {
        std::cout << "I say B" << std::endl;
    }

    REFLECT()
};

REFLECTION_REGISTRATION(B)
{
    CLASS->addField("ala", &B::ala)
        ->addField("bla", &B::bla);
}

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
    float cubesRotationDir;
    float cubesRotation;
    bool paused = false;
    A a;
    B b;

    GameState()
        : reloadablesAlloc{megabytes(4), this + sizeof(this)},
          shaderManager{&reloadablesAlloc},
          cubesRotationDir{1},
          cameraPos{glm::vec3{-1,0,0}},
          cameraForward{glm::vec3{0,0,-1}},
          cameraYaw{-90.0f},
          cameraPitch{0.0f},
          a{},
          b{}
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
        ->addField("cubesRotationDir", &GameState::cubesRotationDir)
        ->addField("cubesRotation", &GameState::cubesRotation)
        ->addField("paused", &GameState::paused)
        ->addField("a", &GameState::a)
        ->addField("b", &GameState::b);
}

static void init(GameState *state) {
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
    glGenVertexArrays(1, &state->VAO);
    glGenBuffers(1, &state->VBO);

    glBindVertexArray(state->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, state->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeModel), cubeModel, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &state->texture);
    glBindTexture(GL_TEXTURE_2D, state->texture);
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


    Shader *bla = state->shaderManager.setShader("bla", vertexShaderSrc, fragmentShaderSrc);
    bla->use();
}

static void update(GameState *state, PlayerInput* input, float dt, float time) {
    if(input->pause) {
        state->paused = !state->paused;
    }

    if(state->paused) {
        return;
    }

    state->cameraYaw += input->mouseDeltaX * 2.0f * dt;
    if(state->cameraYaw > 360) {
        state->cameraYaw -= 360;
    }
    else if(state->cameraYaw < 0) {
        state->cameraYaw += 360;
    }
    state->cameraPitch += -input->mouseDeltaY * 2.0f * dt;
    if(state->cameraPitch > 360) {
        state->cameraPitch -= 360;
    }
    else if(state->cameraPitch < 0) {
        state->cameraPitch += 360;
    }
    state->cameraForward.x = glm::cos(glm::radians(state->cameraYaw)) * glm::cos(glm::radians(state->cameraPitch));
    state->cameraForward.y = glm::sin(glm::radians(state->cameraPitch));
    state->cameraForward.z = glm::sin(glm::radians(state->cameraYaw)) * glm::cos(glm::radians(state->cameraPitch));
    state->cameraForward = glm::normalize(state->cameraForward);

    if(input->horizontal != 0) {
        state->cameraPos += glm::cross(state->cameraForward, glm::vec3{0,1,0}) * 3.0f * (float)input->horizontal * dt;
        state->a.say();
        std::cout << state->a.some();
        state->b.say();
        std::cout << state->b.some();
        A* reallyB = &state->b;
        reallyB->say();
        std::cout << reallyB->some();
        std::cout << "\n-------\n";
    }

    if(input->vertical != 0)
        state->cameraPos += state->cameraForward * 4.0f * (float)input->vertical * dt;
    
    /*if(input->horizontal * state->cubesRotationDir < 0)
        state->cubesRotationDir += 10 * (state->cubesRotationDir >= 0 ? -dt : dt);
    else if(input->horizontal * state->cubesRotationDir > 0 || input->horizontal != 0 && state->cubesRotationDir == 0)
        state->cubesRotationDir -= 2 * (state->cubesRotationDir >= 0 ? -dt : dt);*/
    
    state->cubesRotation += 5 * glm::sin(10 * glm::radians(time)) * state->cubesRotationDir * dt;
}

static void render(GameState *state, float deltaInterpolation) {
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    Shader* bla = state->shaderManager.getShader("bla");
    bla->use();
    bla->setMat4("view", glm::lookAt(state->cameraPos, state->cameraPos + state->cameraForward, glm::vec3{0,1,0}));
    bla->setMat4("projection", projection);

    float color1 = std::fabsf(std::sinf(3.1415f * (state->cubesRotation * 0.5f + 0.5f)));

    bla->setFloat("Color", color1);

    glm::vec3 cubePositions[] {
        glm::vec3{2.0f, 0.0f, 0.0f},
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
        localToWorld = glm::rotate(localToWorld, state->cubesRotation * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
        bla->setMat4("model", localToWorld);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}