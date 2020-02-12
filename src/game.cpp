#include <stdio.h>

#include "common.h"

#include "shaderManager.cpp"
#include "stb_image.cpp"

#define DLLEXPORT extern "C" __declspec(dllexport)

static unsigned int VAO[2];
static unsigned int VBO[2];

static float vertices[] {
        -1.0f, -0.5f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.5f, 1.0f,
        -0.75f, 0.5f, 0.0f, 1.0f, 0.0f,
    };

static float vertices2[] {
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    0.0f, -0.5f, 0.0f, 0.5f, 1.0f,
    -0.25f, 0.5f, 0.0f, 1.0f, 0.0f,
};

static unsigned int texture;

struct GameState {
    float vertices[15];
    float vertices2[15];
};

DLLEXPORT void onLoad(GameMemory* gameMemory) {
    gladLoadGL();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("Untitled.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
    }
    stbi_image_free(data);

    //Create VAO
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);

    //TRIANGLE 1
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);  
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //TRIANGLE 2
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);  
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

DLLEXPORT void start(GameMemory* gameMemory) {
    GameState* gameState = (GameState*)gameMemory->data;

    for(int i = 0; i < 15; i++) {
        gameState->vertices[i] = vertices[i];
        gameState->vertices2[i] = vertices2[i];
    }
}

DLLEXPORT void update(GameMemory* gameMemory) {
    GameState* gameState = (GameState*)gameMemory->data;
    
    for(int i = 0; i < 15; i++) {
        if(i % 5 == 0)
            gameState->vertices[i] += 0.003f;
    }

    for(int i = 0; i < 15; i++) {
        if(i % 5 == 0)
            gameState->vertices2[i] += 0.002f;
    }

    //std::cout << gameState->a.getA() << "\n" << b->getA() << "\nBsA:" << gameState->b.getA() << std::endl;
}

DLLEXPORT void render(GameMemory* gameMemory) {
    GameState* gameState = (GameState*)gameMemory->data;
    Shader* shader = gameMemory->shaderManager->getShader("bla");

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if(shader) {
        shader->use();
        shader->setFloat("Color", 0.2f);
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gameState->vertices), gameState->vertices, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 5);

    if(shader)
        shader->setFloat("Color", 0.7f);
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gameState->vertices2), gameState->vertices2, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 5);
}