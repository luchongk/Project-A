
#include "render.h"
#include "obj_loader.h"
#include "matrix.h"

struct GlobalUniforms {
    Matrix projection;
};

struct PerFrameUniforms {
    Matrix view;
    Vec3 view_pos;
    float padding;
    struct {
        Vec3 position;
        float padding;
        Vec3 ambient;
        float padding2;
        Vec3 diffuse;
        float padding3;
        Vec3 specular;
    } light;
    float time;
};

struct PerObjectUniforms {
    Matrix world;
    struct {
        Vec3 ambient;
        float padding;
        Vec3 diffuse;
        float padding2;
        Vec3 specular;
        float shininess;
    } material;
    Vec3 pos;
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

GlobalUniforms global_uniforms;
PerFrameUniforms  per_frame_uniforms;
PerObjectUniforms per_object_uniforms;

static Texture* grid_texture;

static void adjust_projection(int width, int height) {
    global_uniforms.projection = perspective(width, height);
    modify_buffer(global_uniform_buffer, sizeof(global_uniforms), &global_uniforms);
}

void init_renderer(OSWindow* window) {
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
    adjust_projection((int)window->size.x, (int)window->size.y);
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

    per_frame_uniforms.view_pos = camera.position;

    per_frame_uniforms.view = lookDir(camera.position, camera.forward, Vec3{0,1,0});
    Vec3 test = per_frame_uniforms.view * Vec3{0,0,1};
    
    per_frame_uniforms.light.position = light_pos;
    per_frame_uniforms.light.diffuse = {1.0f, 1.0f, 1.0f};
    per_frame_uniforms.light.ambient = per_frame_uniforms.light.diffuse * Vec3{0.03f, 0.03f, 0.03f};
    per_frame_uniforms.light.specular = {1.0f, 1.0f, 1.0f};
    per_frame_uniforms.time = time.since_start;

    modify_buffer(per_frame_uniform_buffer, sizeof(per_frame_uniforms), &per_frame_uniforms);

    // LIGHT DRAW
    set_shader(light_shader);

    Matrix localToWorld = translation(light_pos);
    localToWorld = scale(localToWorld, 0.2f);

    per_object_uniforms.world = localToWorld;
    modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);

    set_vertex_buffer(cube_vertex_buffer);
    set_index_buffer(cube_index_buffer);
    draw_indexed((uint)cube_mesh.indices.count);
    
    // CUBES DRAW
    set_shader(pbr_shader);

    per_object_uniforms.material.ambient   = {0.49f, 0.37f, 0.11f};
    per_object_uniforms.material.diffuse   = {0.5f, 0.35f, 0.05f};
    per_object_uniforms.material.specular  = {1.0f, 0.782f, 0.17f};
    per_object_uniforms.material.shininess = 16.0f;

    Vec3 cubeposition[] {
        Vec3{0.0f, 0.0f, 0.0f},
        Vec3{2.0f, 5.0f, -15.0f},
        Vec3{-1.5f, -2.2f, -2.5f},
        Vec3{-3.8f, -2.0f, -12.3f},
        Vec3{2.4f, -0.4f, -3.5f},
    };

    set_vertex_buffer(weird_vertex_buffer);
    set_index_buffer(weird_index_buffer);
    for(int i = 0; i < 1; i++) {
        per_object_uniforms.pos = cubeposition[i] + cubes_offset;
        localToWorld = translation(per_object_uniforms.pos);
        localToWorld = rotate(localToWorld, {0,1,0}, cubes_rotation);
        per_object_uniforms.world = localToWorld;
        modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);

        draw_indexed((uint)weird_mesh.indices.count);
    }

    os_swap_buffers(window);
}

void end_renderer() {
    release_texture(grid_texture);
}