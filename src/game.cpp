#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "game.h"

#ifdef DEBUG
#include "debug.cpp"
#endif

#include "shader.cpp"
#include "stb_image.cpp"

/**
 * TODO:
 * * Clean up vector.h
 * * Array type
 * * LearnOpengl
 * * Make Debug UI
 * * Physics
 */

#define DLLEXPORT extern "C" __declspec(dllexport)

DLLEXPORT void onLoad(bool is_init, GameMemory *game_memory, PlatformAPI* platform_api) {
    gladLoadGL();
    platform = platform_api;
    game_state = (GlobalState*)game_memory->data;
    default_allocator_data = &game_state->temporary_memory;

    if(is_init) {
        *game_state = {};
        init_arena(&game_state->temporary_memory, megabytes(16), (void*)(game_state + 1));

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
        glGenVertexArrays(1, &game_state->VAO);
        glGenBuffers(1, &game_state->VBO);

        glBindVertexArray(game_state->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, game_state->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeModel), cubeModel, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glGenTextures(1, &game_state->texture);
        glBindTexture(GL_TEXTURE_2D, game_state->texture);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load and generate the texture
        int width, height, nrChannels;
        unsigned char *image_data = stbi_load("assets\\textures\\Untitled.png", &width, &height, &nrChannels, 0);
        if (image_data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            printf("Failed to load texture\n");
            fflush(stdout);
        }
        stbi_image_free(image_data);
    }

    if(!is_init) delete_shader(&game_state->shader);
    compile_shader(&game_state->shader, "bla", "assets/shaders/basic.vert", "assets/shaders/basic.frag");
    use_shader(&game_state->shader);
}

DLLEXPORT void update(GameMemory *game_memory, PlayerInput* input, float dt, float time) {
    game_state->lastCameraPos = game_state->cameraPos;
    
    if(input->pause) {
        game_state->paused = !game_state->paused;
    }

    if(game_state->paused) {
        return;
    }

    game_state->cameraYaw += input->mouseDeltaX * 2.0f * dt;
    if(game_state->cameraYaw > 360) {
        game_state->cameraYaw -= 360;
    }
    else if(game_state->cameraYaw < 0) {
        game_state->cameraYaw += 360;
    }
    game_state->cameraPitch += -input->mouseDeltaY * 2.0f * dt;
    if(game_state->cameraPitch > 360) {
        game_state->cameraPitch -= 360;
    }
    else if(game_state->cameraPitch < 0) {
        game_state->cameraPitch += 360;
    }
    game_state->cameraForward.x = glm::cos(glm::radians(game_state->cameraYaw)) * glm::cos(glm::radians(game_state->cameraPitch));
    game_state->cameraForward.y = glm::sin(glm::radians(game_state->cameraPitch));
    game_state->cameraForward.z = glm::sin(glm::radians(game_state->cameraYaw)) * glm::cos(glm::radians(game_state->cameraPitch));
    game_state->cameraForward.normalize();

    if(input->horizontal != 0) {
        game_state->cameraPos += game_state->cameraForward.cross(Vector3{0,1,0}) * 3.0f * (float)input->horizontal * dt;
    }

    if(input->vertical != 0) {
        game_state->cameraPos += game_state->cameraForward * 2.0f * (float)input->vertical * dt;
    }
    
    game_state->cubesRotation += 2 * glm::sin(10 * glm::radians(time)) * game_state->cubesRotationDir * dt;
}

DLLEXPORT void render(GameMemory *game_memory, float deltaInterpolation) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(game_state->cameraPos.toGLM(), (game_state->cameraPos + game_state->cameraForward).toGLM(), glm::vec3{0,1,0});
    
    set_shader_uniform(&game_state->shader, "view", view);
    set_shader_uniform(&game_state->shader, "projection", projection);

    glm::vec3 cubePositions[] {
        glm::vec3{2.0f, 0.0f, 0.0f},
        glm::vec3{2.0f, 5.0f, -15.0f},
        glm::vec3{-1.5f, -2.2f, -2.5f},
        glm::vec3{-3.8f, -2.0f, -12.3f},
        glm::vec3{2.4f, -0.4f, -3.5f},
    };

    glBindVertexArray(game_state->VAO);
    for(int i = 0; i < 5; i++) {
        glm::mat4 localToWorld = glm::translate(glm::mat4{1.0f}, cubePositions[i]);
        float angle = 50.0f * (i+1);
        localToWorld = glm::rotate(localToWorld, game_state->cubesRotation * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
        
        set_shader_uniform(&game_state->shader, "model", localToWorld);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    //Free temporary memory
    freeAll();
}