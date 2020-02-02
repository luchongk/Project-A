#include <windows.h>
#include <stdio.h>
#include "glad/glad.h"

#define DLLEXPORT extern "C" __declspec(dllexport)

static unsigned int VAO[2];
static unsigned int VBO[2];

static float vertices[]{
        -0.5f, -0.5f, 0.0f,
        0.0f, -0.5f, 0.0f,
        -0.25f, 0.5f, 0.0f,
    };

static float vertices2[] = {
    0.0f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.25f, 0.5f, 0.0f,
};

DLLEXPORT void init() {
    gladLoadGL();
    
    //Create VAO
    glGenVertexArrays(2, VAO);

    glGenBuffers(2, VBO);

    //TRIANGLE 1
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //TRIANGLE 2
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

DLLEXPORT void update() {
    printf("So happy right now!\n");

    for(int i = 0; i < 9; i++) {
        if(i % 3 == 0)
            vertices[i] += 0.025f;
    }

    for(int i = 0; i < 9; i++) {
        if(i % 3 == 0)
            vertices2[i] += 0.01f;
    }

    vertices[7] += 0.01f;

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
}

DLLEXPORT void render(unsigned int shader1, unsigned int shader2) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader1);
    
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUseProgram(shader2);
    
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}