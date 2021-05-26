
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
    Vec3 position;
    float padding;
};

Mesh weird_mesh;
Mesh cube_mesh;

uint pbr_shader;
uint light_shader;

static GraphicsBuffer* weird_vertex_buffer;
static GraphicsBuffer* weird_index_buffer;
static GraphicsBuffer* cube_vertex_buffer;
static GraphicsBuffer* cube_index_buffer;

static GraphicsBuffer* global_uniform_buffer;
static GraphicsBuffer* per_frame_uniform_buffer;
static GraphicsBuffer* per_object_uniform_buffer;

static GlobalUniforms global_uniforms;
static PerFrameUniforms  per_frame_uniforms;
static PerObjectUniforms per_object_uniforms;

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
    per_frame_uniforms.view = look_dir(camera.position, camera.forward, Vec3{0,1,0});
    
    per_frame_uniforms.light.position = light.position;
    per_frame_uniforms.light.diffuse = light.diffuse;
    per_frame_uniforms.light.ambient = light.ambient;
    per_frame_uniforms.light.specular = light.specular;
    
    per_frame_uniforms.time = time.since_start;

    modify_buffer(per_frame_uniform_buffer, sizeof(per_frame_uniforms), &per_frame_uniforms);

    // LIGHT DRAW
    set_shader(light_shader);

    Matrix localToWorld = translation(light.position);
    localToWorld = scale(localToWorld, 0.2f);

    per_object_uniforms.world = localToWorld;
    modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);

    set_vertex_buffer(cube_vertex_buffer);
    set_index_buffer(cube_index_buffer);
    draw_indexed((uint)light.mesh->indices.count);
    
    // CUBES DRAW
    set_shader(pbr_shader);

    set_vertex_buffer(weird_vertex_buffer);
    set_index_buffer(weird_index_buffer);
    for(int i = 0; i < 5; i++) {
        per_object_uniforms.material.ambient   = entities[i].material.ambient;
        per_object_uniforms.material.diffuse   = entities[i].material.diffuse;
        per_object_uniforms.material.specular  = entities[i].material.specular;
        per_object_uniforms.material.shininess = entities[i].material.shininess;
        per_object_uniforms.position = entities[i].position;
        
        per_object_uniforms.world = get_world_matrix(&entities[i]);

        modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);
        draw_indexed((uint)entities[i].mesh->indices.count);
    }

    os_swap_buffers(window);
}

void end_renderer() {
    release_texture(grid_texture);
}