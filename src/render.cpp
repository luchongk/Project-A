
#include "render.h"
#include "obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

/*static uint VAO;
static uint light_VAO;
static uint VBO;
static uint light_VBO;
static uint EBO;
static uint light_EBO;
static uint texture;
static Shader shader;
*/

struct GlobalUniforms {
    glm::mat4 projection;
};

struct PerFrameUniforms {
    glm::mat4 view;
    glm::vec3 view_pos;
    float padding;
    struct {
        glm::vec3 position;
        float padding;
        glm::vec3 ambient;
        float padding2;
        glm::vec3 diffuse;
        float padding3;
        glm::vec3 specular;
    } light;
    float time;
};

struct PerObjectUniforms {
    glm::mat4 world;
    struct {
        glm::vec3 ambient;
        float padding;
        glm::vec3 diffuse;
        float padding2;
        glm::vec3 specular;
        float shininess;
    } material;
    glm::vec3 pos;
    float padding;
};

static Mesh weird_mesh;
static Mesh cube_mesh;

static uint pbr_shader;
static uint light_shader;

static GraphicsBuffer* weird_vertex_buffer;
static GraphicsBuffer* weird_index_buffer;
static GraphicsBuffer* cube_vertex_buffer;
static GraphicsBuffer* cube_index_buffer;

static GraphicsBuffer* global_uniform_buffer;
static GraphicsBuffer* per_frame_uniform_buffer;
static GraphicsBuffer* per_object_uniform_buffer;

static Texture* grid_texture;

GlobalUniforms global_uniforms;
PerFrameUniforms  per_frame_uniforms;
PerObjectUniforms per_object_uniforms;

static void adjust_projection(int width, int height) {
    // @Journey 04/13/2021: We learned that Direct3D NDC coordinates go from 0 (near plane) to 1 (far plane) in the z-axis.
    // This means we can't rely on glm::perspective to create a projection matrix for us because OpenGL's convention
    // is to map z coordinates to values between -1 (near plane) and 1 (far plane). So.. we just learned how to make our own projection matrix!
    // X and Y axis are treated the same in D3D vs OpenGL (-1 to 1), so we borrowed those rows from glm's matrix.

    const float aspect = (float)width / height;
    const float tanHalfFovy = tan(glm::radians(45.0f) / 2);
    global_uniforms.projection = glm::mat4{
        1.0f / (aspect * tanHalfFovy),               0.0f,          0.0f,                0.0f,
                                 0.0f, 1.0f / tanHalfFovy,          0.0f,                0.0f,
                                 0.0f,               0.0f, -100.0f/99.9f, -100.0 * 0.1f/99.9f,
                                 0.0f,               0.0f,            -1,                0.0f
    };

    modify_buffer(global_uniform_buffer, sizeof(global_uniforms), &global_uniforms);
}

void init_renderer() {
    pbr_shader   = compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, "assets\\shaders\\basic_pixel.hlsl"_s, VertexFormat::PNU);
    light_shader = compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, "assets\\shaders\\light_cube_pixel.hlsl"_s, VertexFormat::PNU);
    set_shader(pbr_shader);

    load_obj("assets\\models\\weirdchamp_smooth.obj"_s, &weird_mesh);

    Array<VertexPNU> vbo_buffer;
    for(int i = 0; i < weird_mesh.vertices.count; i++) {
        array_add(&vbo_buffer, {weird_mesh.vertices[i], weird_mesh.normals[i], weird_mesh.uvs[i]});
    }
    
    weird_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::STATIC, VertexFormat::PNU, (uint)vbo_buffer.count, vbo_buffer.data);
    weird_index_buffer  = create_index_buffer(GraphicsBufferUsage::STATIC, (uint)weird_mesh.indices.count, weird_mesh.indices.data);
    
    load_obj("assets\\models\\cube.obj"_s, &cube_mesh);

    vbo_buffer.count = 0;
    for(int i = 0; i < cube_mesh.vertices.count; i++) {
        array_add(&vbo_buffer, {cube_mesh.vertices[i], cube_mesh.normals[i], cube_mesh.uvs[i]});
    }
    
    cube_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::STATIC, VertexFormat::PNU, (uint)vbo_buffer.count, vbo_buffer.data);
    cube_index_buffer  = create_index_buffer(GraphicsBufferUsage::STATIC, (uint)cube_mesh.indices.count, cube_mesh.indices.data);
    
    global_uniform_buffer = create_uniform_buffer(GraphicsBufferUsage::STATIC, sizeof(GlobalUniforms), nullptr);
    adjust_projection(1600, 800);
    on_size_adjusted = adjust_projection;

    per_frame_uniform_buffer  = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, sizeof(PerFrameUniforms));
    per_object_uniform_buffer = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, sizeof(PerObjectUniforms));

    set_uniform_buffer(UniformBufferSlot::PER_SETTINGS, global_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_FRAME, per_frame_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_OBJECT, per_object_uniform_buffer);

    set_primitive_type(GraphicsPrimitiveType::TRIANGLE);

    grid_texture = create_texture("assets\\textures\\uv_grid.png"_s);
    set_texture(0, grid_texture);
}

void render(OSWindow* window) {
    bind_framebuffer();
    auto srgb = powf(0.1f, 1.0f / 2.2f);
    clear_color_buffer(srgb, srgb, srgb);
    clear_depth_buffer();

    per_frame_uniforms.view_pos = camera.position.toGLM();

    per_frame_uniforms.view = glm::transpose(glm::lookAt(
        per_frame_uniforms.view_pos,
        (camera.position + camera.forward).toGLM(),
        glm::vec3{0,1,0}
    ));
    
    per_frame_uniforms.light.position = light_pos.toGLM();
    per_frame_uniforms.light.diffuse = {1.0f, 1.0f, 1.0f};
    per_frame_uniforms.light.ambient = per_frame_uniforms.light.diffuse * glm::vec3{0.03f};
    per_frame_uniforms.light.specular = {1.0f, 1.0f, 1.0f};
    per_frame_uniforms.time = time.since_start;

    modify_buffer(per_frame_uniform_buffer, sizeof(per_frame_uniforms), &per_frame_uniforms);

    // LIGHT DRAW
    set_shader(light_shader);

    glm::mat4 localToWorld = glm::translate(glm::mat4{1.0f}, light_pos.toGLM());
    localToWorld = glm::scale(localToWorld, glm::vec3{0.2f});

    per_object_uniforms.world = glm::transpose(localToWorld);
    modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);

    set_vertex_buffer(cube_vertex_buffer);
    set_index_buffer(cube_index_buffer);
    draw_indexed((uint)cube_mesh.indices.count);
    
    // CUBES DRAW
    set_shader(pbr_shader);

    per_object_uniforms.material.ambient   = {0.49f, 0.37f, 0.11f};
    per_object_uniforms.material.diffuse   = {0.0f, 0.0f, 0.0f};
    per_object_uniforms.material.specular  = {1.0f, 0.782f, 0.17f};
    per_object_uniforms.material.shininess = 16.0f;

    glm::vec3 cubeposition[] {
        glm::vec3{0.0f, 0.0f, 0.0f},
        glm::vec3{2.0f, 5.0f, -15.0f},
        glm::vec3{-1.5f, -2.2f, -2.5f},
        glm::vec3{-3.8f, -2.0f, -12.3f},
        glm::vec3{2.4f, -0.4f, -3.5f},
    };

    set_vertex_buffer(weird_vertex_buffer);
    set_index_buffer(weird_index_buffer);
    for(int i = 0; i < 1; i++) {
        localToWorld = glm::translate(glm::mat4{1.0f}, cubeposition[i] + cubes_offset.toGLM());
        per_object_uniforms.pos = cubeposition[i] + cubes_offset.toGLM();
        float angle = cubes_rotation;//50.0f * (i+1);
        localToWorld = glm::rotate(localToWorld, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        per_object_uniforms.world = glm::transpose(localToWorld);
        modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);

        draw_indexed((uint)weird_mesh.indices.count);
    }

    os_swap_buffers(window);
}

void end_renderer() {
    release_texture(grid_texture);
}