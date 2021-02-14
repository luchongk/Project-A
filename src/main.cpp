#include "platform.h"

#ifdef _WIN32
    #include "platform_win32.cpp"
#endif

#include "stb_image.cpp"
#include "shader.cpp"

struct Time {
    s64 start_stamp;
    s64 last_stamp;
    s64 frequency;
    float since_start;
    float delta;
};

struct PlayerInput {
    int horizontal;
    int vertical;
    Vector2 mouse_delta;
};

struct Camera {
    Vector3 position;
    Vector3 forward = {0, 0, -1};
    float yaw = -90;
    float pitch;
};

Time time;
float fixedDeltaTime = 1.0f/60;
float maxFrameTime = 0.25f;

uint VAO;
uint light_VAO;
uint VBO;
uint texture;
Shader shader;
Shader light_shader;
Camera camera;
Vector3 light_pos = {1.0f, 1.0f, -6.0f};
float cubesRotationDir = 1;
float cubesRotation = 0;
bool paused = true;
float light_time_accum = 0.0f;

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

    glGenBuffers(1, &VBO);

    //Create cube VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeModel), cubeModel, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Create light VAO
    glGenVertexArrays(1, &light_VAO);
    glBindVertexArray(light_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenTextures(1, &texture);
    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
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

    compile_shader(&shader, "basic", "assets/shaders/basic.vert", "assets/shaders/basic.frag");
    compile_shader(&light_shader, "light_shader", "assets/shaders/basic.vert", "assets/shaders/light_cube.frag");
}

void update_time() {
    s64 now = os_get_timestamp();
    
    time.delta = (now - time.last_stamp) / (float)time.frequency;
    if(time.delta > maxFrameTime) {
        //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
        time.delta = maxFrameTime;
    }

    if(paused) time.start_stamp += now - time.last_stamp;   //Skip time if paused
    time.since_start = (now - time.start_stamp) / (float)time.frequency;
    time.last_stamp = now;
}

void handle_events(OSWindow* window, PlayerInput* input) {
    input->mouse_delta = os_events.mouse_delta;

    For(os_events.keyboard) {
        if(it->pressed != it->wasDown) {
            switch(it->code) {
                case 'A': {
                    input->horizontal += it->pressed ? -1 : 1;
                    break;
                }

                case 'D': {
                    input->horizontal += it->pressed ? 1 : -1;
                    break;
                }

                case 'S': {
                    input->vertical += it->pressed ? -1 : 1;
                    break;
                }

                case 'W': {
                    input->vertical += it->pressed ? 1 : -1;
                    break;
                }

                default: {
                    if(it->pressed) {
                        switch(it->code) {
                            case 'P': {
                                paused = !paused;
                                break;
                            }

                            case 'R': {
                                camera = {};
                                cubesRotation = 0;
                                light_time_accum = 0;
                                break;
                            }

                            case VK_F11: {
                                os_toggle_fullscreen(window, true);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

static void update_camera(PlayerInput* input, float dt) {
    camera.yaw += input->mouse_delta.x * 2.0f * dt;
    if(camera.yaw > 360) {
        camera.yaw -= 360;
    }
    else if(camera.yaw < 0) {
        camera.yaw += 360;
    }
    camera.pitch += -input->mouse_delta.y * 2.0f * dt;
    if(camera.pitch > 360) {
        camera.pitch -= 360;
    }
    else if(camera.pitch < 0) {
        camera.pitch += 360;
    }

    camera.forward.x = glm::cos(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
    camera.forward.y = glm::sin(glm::radians(camera.pitch));
    camera.forward.z = glm::sin(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
    camera.forward.normalize();

    if(input->horizontal != 0) {
        camera.position += camera.forward.cross(Vector3{0,1,0}) * 3.0f * (float)input->horizontal * dt;
    }

    if(input->vertical != 0) {
        camera.position += camera.forward * 2.0f * (float)input->vertical * dt;
    }
}

void simulate(PlayerInput* input) {
    if(paused) {
        return;
    }

    update_camera(input, time.delta);
    
    light_time_accum += time.delta;
    
    cubesRotation += 2 * glm::sin(10 * glm::radians(light_time_accum)) * cubesRotationDir * time.delta;
}

void render(OSWindow* window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        camera.position.toGLM(),
        (camera.position + camera.forward).toGLM(),
        glm::vec3{0,1,0}
    );

    // LIGHT DRAW
    light_pos = {
        glm::cos(light_time_accum * 0.75f) * 2,
        glm::sin(light_time_accum * 0.75f) * 2, -6.0f
    };

    use_shader(&light_shader);
    set_shader_uniform(&light_shader, "view", view);
    set_shader_uniform(&light_shader, "projection", projection);

    glm::mat4 localToWorld = glm::translate(glm::mat4{1.0f}, light_pos.toGLM());
    localToWorld = glm::scale(localToWorld, glm::vec3(0.2f));
    set_shader_uniform(&light_shader, "model", localToWorld);

    glBindVertexArray(light_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    
    // CUBES DRAW
    use_shader(&shader);
    set_shader_uniform(&shader, "view", view);
    set_shader_uniform(&shader, "projection", projection);
    set_shader_uniform(&shader, "objectColor", glm::vec3{0.0f, 0.3f, 0.5f});
    set_shader_uniform(&shader, "lightColor",  glm::vec3{1.0f, 1.0f, 1.0f});
    set_shader_uniform(&shader, "lightPos", light_pos.toGLM());
    set_shader_uniform(&shader, "viewPos", camera.position.toGLM());

    glm::vec3 cubepositionitions[] {
        glm::vec3{2.0f, 0.0f, 0.0f},
        glm::vec3{2.0f, 5.0f, -15.0f},
        glm::vec3{-1.5f, -2.2f, -2.5f},
        glm::vec3{-3.8f, -2.0f, -12.3f},
        glm::vec3{2.4f, -0.4f, -3.5f},
    };

    glBindVertexArray(VAO);
    for(int i = 0; i < 5; i++) {
        localToWorld = glm::translate(glm::mat4{1.0f}, cubepositionitions[i]);
        float angle = 50.0f * (i+1);
        localToWorld = glm::rotate(localToWorld, cubesRotation * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
        set_shader_uniform(&shader, "model", localToWorld);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    os_swap_buffers(window);
}

void main() {
    default_allocator = malloc_allocator;

    PlayerInput input{};  //Not sure where to put this, I'll leave it there for now.

    OSWindow* window = os_create_window();

    time.start_stamp = os_get_timestamp();
    time.frequency = os_get_timer_frequency();

    float accum = 0;

    init();

    bool exit = false;
    while(!exit) {
        update_time();

        exit = os_poll_events();

        handle_events(window, &input);
        
        accum += time.delta;
        while(accum >= fixedDeltaTime) {
            simulate(&input);
            accum -= fixedDeltaTime;
        }

        render(window);

#if 0
    printf_s("last frame: %f ms\n", time.delta * 1000);
    printf_s("FPS: %f\n", 1 / time.delta);
    fflush(stdout);   //CPU usage goes x10 and my laptops fan starts going crazy if we do this every frame. WTF!
#endif
    }
}