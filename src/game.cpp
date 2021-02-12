#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "types.h"
#include "array.h"
#include "game.h"

#include "debug.cpp"
#include "shader.cpp"
#include "stb_image.cpp"

static void update_camera(Camera* camera, PlayerInput* input, float dt) {
    camera->yaw += input->mouseDeltaX * 2.0f * dt;
    if(camera->yaw > 360) {
        camera->yaw -= 360;
    }
    else if(camera->yaw < 0) {
        camera->yaw += 360;
    }
    camera->pitch += -input->mouseDeltaY * 2.0f * dt;
    if(camera->pitch > 360) {
        camera->pitch -= 360;
    }
    else if(camera->pitch < 0) {
        camera->pitch += 360;
    }
    camera->forward.x = glm::cos(glm::radians(camera->yaw)) * glm::cos(glm::radians(camera->pitch));
    camera->forward.y = glm::sin(glm::radians(camera->pitch));
    camera->forward.z = glm::sin(glm::radians(camera->yaw)) * glm::cos(glm::radians(camera->pitch));
    camera->forward.normalize();

    if(input->horizontal != 0) {
        camera->position += camera->forward.cross(Vector3{0,1,0}) * 3.0f * (float)input->horizontal * dt;
    }

    if(input->vertical != 0) {
        camera->position += camera->forward * 2.0f * (float)input->vertical * dt;
    }
}

#define DLLEXPORT extern "C" __declspec(dllexport)

DLLEXPORT void onLoad(bool is_init, GameMemory *game_memory, PlatformAPI* platform_api) {
    gladLoadGL();
    platform = platform_api;
    game_state = (GlobalState*)game_memory->data;
    default_allocator = linear_allocator;
    default_allocator_data = &game_state->temporary_memory;

    if(is_init) {
        *game_state = {};
        init_arena(&game_state->temporary_memory, megabytes(16), (void*)(game_state + 1));

        glEnable(GL_DEPTH_TEST);

        float cubeModel[] {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.4f, 0.4f,
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.6f, 0.4f,
            0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.6f, 0.6f,
            0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.6f, 0.6f,
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.4f, 0.6f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.4f, 0.4f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
        };

        glGenBuffers(1, &game_state->VBO);

        //Create cube VAO
        glGenVertexArrays(1, &game_state->VAO);
        glBindVertexArray(game_state->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, game_state->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeModel), cubeModel, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        //Create light VAO
        glGenVertexArrays(1, &game_state->lightVAO);
        glBindVertexArray(game_state->lightVAO);

        glBindBuffer(GL_ARRAY_BUFFER, game_state->VBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenTextures(1, &game_state->texture);
        //glActiveTexture(GL_TEXTURE0);
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
    else {
        delete_shader(&game_state->shader);
        delete_shader(&game_state->light_shader);
    }
    
    compile_shader(&game_state->shader, "basic", "assets/shaders/basic.vert", "assets/shaders/basic.frag");
    compile_shader(&game_state->light_shader, "light_shader", "assets/shaders/basic.vert", "assets/shaders/light_cube.frag");
}

DLLEXPORT void update(GameMemory* game_memory, PlayerInput* input, float dt, float time) {
    Array<int> my_array{};
    array_add(&my_array, 31);
    array_add(&my_array, 25);
    array_add(&my_array, 21);

    For(my_array) {
        printf("%d\n", *it);
    }
    
    if(input->pause) {
        game_state->paused = !game_state->paused;
    }

    if(game_state->paused) {
        return;
    }

    update_camera(&game_state->camera, input, dt);
    
    game_state->light_pos = {
        glm::cos(time * 0.75f) * 2,
        glm::sin(time * 0.75f) * 2, -6.0f
    };
    
    game_state->cubesRotation += 2 * glm::sin(10 * glm::radians(time)) * game_state->cubesRotationDir * dt;
}

DLLEXPORT void render(GameMemory *game_memory, float deltaInterpolation) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        game_state->camera.position.toGLM(),
        (game_state->camera.position + game_state->camera.forward).toGLM(),
        glm::vec3{0,1,0}
    );

    // LIGHT DRAW
    use_shader(&game_state->light_shader);
    set_shader_uniform(&game_state->light_shader, "view", view);
    set_shader_uniform(&game_state->light_shader, "projection", projection);

    glm::mat4 localToWorld = glm::translate(glm::mat4{1.0f}, game_state->light_pos.toGLM());
    localToWorld = glm::scale(localToWorld, glm::vec3(0.2f));
    set_shader_uniform(&game_state->light_shader, "model", localToWorld);

    glBindVertexArray(game_state->lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    
    // CUBES DRAW
    use_shader(&game_state->shader);
    set_shader_uniform(&game_state->shader, "view", view);
    set_shader_uniform(&game_state->shader, "projection", projection);
    set_shader_uniform(&game_state->shader, "objectColor", glm::vec3{0.0f, 0.3f, 0.5f});
    set_shader_uniform(&game_state->shader, "lightColor",  glm::vec3{1.0f, 1.0f, 1.0f});
    set_shader_uniform(&game_state->shader, "lightPos", game_state->light_pos.toGLM());
    set_shader_uniform(&game_state->shader, "viewPos", game_state->camera.position.toGLM());

    glm::vec3 cubepositionitions[] {
        glm::vec3{2.0f, 0.0f, 0.0f},
        glm::vec3{2.0f, 5.0f, -15.0f},
        glm::vec3{-1.5f, -2.2f, -2.5f},
        glm::vec3{-3.8f, -2.0f, -12.3f},
        glm::vec3{2.4f, -0.4f, -3.5f},
    };

    glBindVertexArray(game_state->VAO);
    for(int i = 0; i < 5; i++) {
        localToWorld = glm::translate(glm::mat4{1.0f}, cubepositionitions[i]);
        float angle = 50.0f * (i+1);
        localToWorld = glm::rotate(localToWorld, game_state->cubesRotation * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
        set_shader_uniform(&game_state->shader, "model", localToWorld);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    reset(&game_state->temporary_memory);
}