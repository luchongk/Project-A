#include <stdio.h>

#include <windows.h>
#include "glad/glad.h"

#include "shader.cpp"
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

static Shader shader;
static unsigned int texture;

class A {
    int _a;

public:
    A(int a) : _a(a) {};

    virtual int getA() { return _a; }
};

class B : public A {
    int _b;

public:
    B(int b) : A(b), _b(b) {};

    int getB() { return _b; }
};

struct GameMemory {
    A a;
    B b;
    float vertices[15];
    float vertices2[15];
};

DLLEXPORT void onLoad(GameMemory* gameMemory) {
    gladLoadGL();

    char* vertexShaderSrc = 
        "#version 460 core\n"
        "layout (location = 0) in vec3 inPosition;\n"
        "layout (location = 1) in vec2 inTexCoord;\n"
        "out vec3 Position;\n"
        "out vec2 TexCoord;\n\n"
        "void main() {\n"
        "    Position = inPosition;\n"
        "    TexCoord = inTexCoord;\n"
        "    gl_Position = vec4(inPosition.x, inPosition.y, inPosition.z, 1.0f);\n"
        "}";

    char* fragmentShaderSrc = 
        "#version 460 core\n"
        "in vec3 Position;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform float Color;\n"
        "uniform sampler2D s;\n"
        "void main() {\n"
        "    FragColor = texture(s, TexCoord) * vec4(Color, 1.0f - Color, Color, 1.0f);\n"
        "}";

    shader = Shader(vertexShaderSrc, fragmentShaderSrc);

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
        std::cout << "Failed to load texture" << std::endl;
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

static A* a;
static A* b;
DLLEXPORT void start(GameMemory* gameMemory) {
    a = new (&gameMemory->a) A(11);
    b = new (&gameMemory->b) B(22);
    for(int i = 0; i < 15; i++) {
        gameMemory->vertices[i] = vertices[i];
        gameMemory->vertices2[i] = vertices2[i];
    }

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gameMemory->vertices), gameMemory->vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gameMemory->vertices2), gameMemory->vertices2, GL_STATIC_DRAW);
}

DLLEXPORT void update(GameMemory* gameMemory) {
    for(int i = 0; i < 15; i++) {
        if(i % 5 == 0)
            gameMemory->vertices[i] += 0.003f;
    }

    for(int i = 0; i < 15; i++) {
        if(i % 5 == 0)
            gameMemory->vertices2[i] += 0.002f;
    }

    //std::cout << gameMemory->a.getA() << "\n" << b->getA() << "\nBsA:" << gameMemory->b.getA() << std::endl;

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gameMemory->vertices), gameMemory->vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gameMemory->vertices2), gameMemory->vertices2, GL_STATIC_DRAW);
}

DLLEXPORT void render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();
    shader.setFloat("Color", 0.2f);
    
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLES, 0, 5);

    shader.setFloat("Color", 0.7f);
    
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_TRIANGLES, 0, 5);
}