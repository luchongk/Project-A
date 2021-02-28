
#include "render.h"
#include "obj_loader.h"

static uint VAO;
static uint light_VAO;
static uint VBO;
static uint EBO;
static uint texture;
static Shader shader;
static Shader light_shader;
static Mesh mesh;

struct VertexXNU {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

void init() {
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    load_obj("assets\\models\\weirdchamp.obj"_s, &mesh);

    Array<VertexXNU> vbo_buffer;
    for(int i = 0; i < mesh.vertices.count; i++) {
        array_add(&vbo_buffer, {mesh.vertices[i], mesh.normals[i], mesh.uvs[i]});
    }

    glGenBuffers(1, &VBO);

    //Create cube VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vbo_buffer.count * sizeof(VertexXNU), vbo_buffer.data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexXNU), (void *)offsetof(VertexXNU, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexXNU), (void *)offsetof(VertexXNU, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexXNU), (void *)offsetof(VertexXNU, uv));
    glEnableVertexAttribArray(2);
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.count * sizeof(uint), mesh.indices.data, GL_STATIC_DRAW);

    //Create light VAO
    glGenVertexArrays(1, &light_VAO);
    glBindVertexArray(light_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

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

    stbi_set_flip_vertically_on_load(true);  
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *image_data = stbi_load("assets\\textures\\uv_grid.png", &width, &height, &nrChannels, 0);
    if (image_data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
        fflush(stdout);
    }
    stbi_image_free(image_data);

    compile_shader(&shader, "assets/shaders/basic.vert"_s, "assets/shaders/basic.frag"_s);
    compile_shader(&light_shader, "assets/shaders/basic.vert"_s, "assets/shaders/light_cube.frag"_s);
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
        glm::sin(light_time_accum * 0.75f) * 2,
        -6.0f
    };

    glm::vec3 light_color = {0.2f, 0.5f, 0.9f};
    glm::vec3 light_ambient = light_color * glm::vec3{0.4f};

    use_shader(&light_shader);
    set_shader_uniform(&light_shader, "view", view);
    set_shader_uniform(&light_shader, "projection", projection);
    set_shader_uniform(&light_shader, "color", light_color);

    glm::mat4 localToWorld = glm::translate(glm::mat4{1.0f}, light_pos.toGLM());
    localToWorld = glm::scale(localToWorld, glm::vec3(0.2f));
    set_shader_uniform(&light_shader, "model", localToWorld);

    glBindVertexArray(light_VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    
    // CUBES DRAW
    use_shader(&shader);
    set_shader_uniform(&shader, "view", view);
    set_shader_uniform(&shader, "projection", projection);
    set_shader_uniform(&shader, "material.ambient", glm::vec3{0.2f, 0.5f, 0.2f});
    set_shader_uniform(&shader, "material.diffuse", glm::vec3{0.2f, 0.5f, 0.8});
    set_shader_uniform(&shader, "material.specular", glm::vec3{1.0f, 0.0f, 0.8f});
    set_shader_uniform(&shader, "material.shininess", 32.0f);
    set_shader_uniform(&shader, "light.position", light_pos.toGLM());
    set_shader_uniform(&shader, "light.ambient",  light_ambient);
    set_shader_uniform(&shader, "light.diffuse",  light_color);
    set_shader_uniform(&shader, "light.specular",  glm::vec3{1.0f, 1.0f, 1.0f});
    set_shader_uniform(&shader, "view_pos", camera.position.toGLM());

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
        localToWorld = glm::rotate(localToWorld, cubes_rotation * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
        set_shader_uniform(&shader, "model", localToWorld);

        glDrawElements(GL_TRIANGLES, (uint)mesh.indices.count, GL_UNSIGNED_INT, 0);
    }

    os_swap_buffers(window);
}