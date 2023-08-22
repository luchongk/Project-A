
#include "render.h"
#include "obj_loader.h"
#include "matrix.h"
#include "ui.h"
#include "simple_draw.h"

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
};

Mesh weird_mesh;
Mesh cube_mesh;

ShaderProgram* basic_shader;
ShaderProgram* light_shader;
ShaderProgram* debug_shader;

Texture* grid_texture;
Texture* test_texture;

MaterialBasic  MATERIAL_GROUND;
MaterialBasic  MATERIAL_PLAYER;
MaterialBasic  MATERIAL_PLAYER2;
MaterialNoData MATERIAL_LIGHT;

Vec3 background = {0,0.02f,0.08f};

Framebuffer* onscreen_framebuffer;

static GraphicsBuffer* static_vertex_buffer;
static GraphicsBuffer* static_index_buffer;

static GraphicsBuffer* debug_vertex_buffer;

static GraphicsBuffer* global_uniform_buffer;
static GraphicsBuffer* per_frame_uniform_buffer;
static GraphicsBuffer* per_material_uniform_buffer;
static GraphicsBuffer* per_object_uniform_buffer;

static GlobalUniforms    global_uniforms;
static PerFrameUniforms  per_frame_uniforms;
static PerObjectUniforms per_object_uniforms;

void set_projection(int width, int height) {
    global_uniforms.projection = perspective(width, height);
    modify_buffer(global_uniform_buffer, sizeof(global_uniforms), &global_uniforms);
}

void init_renderer(OSWindow* window) {
    onscreen_framebuffer = create_onscreen_framebuffer((int)window->size.x, (int)window->size.y);
    
    basic_shader = compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, "assets\\shaders\\basic_pixel.hlsl"_s, VertexFormat::PNU);
    light_shader = compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, "assets\\shaders\\light_cube_pixel.hlsl"_s, VertexFormat::PNU);
    debug_shader = compile_shader("assets\\shaders\\debug_vertex.hlsl"_s, "assets\\shaders\\ui_pixel.hlsl"_s, VertexFormat::PCU);

    grid_texture = create_texture_from_file("assets\\textures\\uv_grid_white.png"_s);

    // Material initialization
    {
        MATERIAL_GROUND.shader = basic_shader;
        MATERIAL_GROUND.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = white_pixel;
        MATERIAL_GROUND.ambient  = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.diffuse  = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.specular = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.shininess = 32.0f;

        MATERIAL_PLAYER.shader = basic_shader;
        MATERIAL_PLAYER.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = white_pixel;
        MATERIAL_PLAYER.ambient  = {0.0f, 0.0f, 0.0f};
        MATERIAL_PLAYER.diffuse  = {0.0f, 1.0f, 0.0f};
        MATERIAL_PLAYER.specular = {0.0f, 0.0f, 0.0f};
        MATERIAL_PLAYER.shininess = 1.0f;

        MATERIAL_PLAYER2 = MATERIAL_PLAYER;

        MATERIAL_LIGHT.shader = light_shader;
    }

    // Static mesh loading. @Speed: Doing a multiple unnecessary buffer copies
    {
        load_obj("assets\\models\\weirdchamp.obj"_s, &weird_mesh);
        load_obj("assets\\models\\cube.obj"_s, &cube_mesh);

        Array<VertexPNU> vertices;
        vertices.allocator = linear_allocator;
        vertices.allocator_data = &temporary_storage;
        Array<uint> indices;
        indices.allocator = linear_allocator;
        indices.allocator_data = &temporary_storage;

        cube_mesh.vertex_base = vertices.count;
        cube_mesh.index_base = indices.count;
        for(uint i = 0; i < cube_mesh.vertices.count; i++) {
            array_add(&vertices, {cube_mesh.vertices[i], cube_mesh.normals[i], cube_mesh.uvs[i]});
        }

        For(cube_mesh.indices) {
            array_add(&indices, *it);
        }

        weird_mesh.vertex_base = vertices.count;
        weird_mesh.index_base = indices.count;
        for(uint i = 0; i < weird_mesh.vertices.count; i++) {
            array_add(&vertices, {weird_mesh.vertices[i], weird_mesh.normals[i], weird_mesh.uvs[i]});
        }

        For(weird_mesh.indices) {
            array_add(&indices, *it);
        }

        static_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::STATIC, VertexFormat::PNU, vertices.count, vertices.data);
        static_index_buffer  = create_index_buffer(GraphicsBufferUsage::STATIC, indices.count, indices.data);

        debug_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::DYNAMIC, VertexFormat::PCU, 256);
    }
    
    global_uniform_buffer = create_uniform_buffer(GraphicsBufferUsage::STATIC, sizeof(GlobalUniforms));
    set_projection((int)window->size.x, (int)window->size.y);

    per_frame_uniform_buffer    = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, sizeof(PerFrameUniforms));
    per_object_uniform_buffer   = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, sizeof(PerObjectUniforms));
    per_material_uniform_buffer = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, 160);  //@Cleanup :Hardcoded: Hardcoded size. Figure out a good one.

    set_uniform_buffer(UniformBufferSlot::PER_SETTINGS, global_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_FRAME,    per_frame_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_MATERIAL, per_material_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_OBJECT,   per_object_uniform_buffer);

    set_primitive_type(GraphicsPrimitiveType::TRIANGLE);

    ui_init();
}

void render(OSWindow* window) {
    bind_framebuffer(onscreen_framebuffer);

    set_blend(false);
    set_depth(true);
    
    clear_color_buffer(background.r, background.g, background.b);
    clear_depth_buffer();
    
    per_frame_uniforms.light.position = light->entity->position;
    per_frame_uniforms.light.diffuse  = light->diffuse;
    per_frame_uniforms.light.ambient  = light->ambient;
    per_frame_uniforms.light.specular = light->specular;
    per_frame_uniforms.time = my_time.since_start;
    modify_buffer(per_frame_uniform_buffer, sizeof(per_frame_uniforms), &per_frame_uniforms);

    set_vertex_buffer(static_vertex_buffer);
    set_index_buffer(static_index_buffer);

    for(int i = 0; i < entity_count; i++) {
        Entity* e = &entities[i];

        if(!e->mesh || !e->material) continue;
        
        Material* material = e->material;
        if(material->shader == nullptr) {
            printf("Tried to draw an entity with null shader\n");
            continue;
        }

        //@Speed: Sort by shader/material instead!
        if(current_shader != material->shader) {
            set_shader(material->shader);
        }
            
        for(uint j = 0; j < material->shader->pixel_texture_slots.count; j++) {
            set_texture(material->shader->pixel_texture_slots[j], material->pixel_textures[j]);
        }

        if(material->constants_size > 0) {
            assert(material->constants_size < 160); // See :Hardcoded
            modify_buffer(per_material_uniform_buffer, material->constants_size, get_material_constants(material));
        }

        per_object_uniforms.world = get_world_matrix(e);
        modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);

        draw_indexed(e->mesh->vertex_base, e->mesh->index_base, e->mesh->indices.count);
    }

#if 0
    // TODO: This is a good example of something we could do in a geometry shader, whenever we support those anyways...
    set_primitive_type(GraphicsPrimitiveType::LINE);
    set_vertex_buffer(debug_vertex_buffer);
    set_shader(debug_shader);
    Array<VertexPCU> vertices;
    for(int i = 0; i < entity_count; i++) {
        array_reset(&vertices);
        
        Entity* entity = &entities[i];
        if(!entity->mesh) continue;
        per_object_uniforms.world = get_world_matrix(&entities[i]);
        
        for(uint j = 0; j < entity->mesh->indices.count; j++) {
            uint index = entity->mesh->indices[j];
            VertexPCU v;
            v.color = {1,0,0,1};
            v.position = entity->mesh->vertices[index];
            array_add(&vertices, v);

            v.color = {1,0,0,1};
            v.position = entity->mesh->vertices[index] + 0.25f * entity->mesh->normals[index];
            array_add(&vertices, v);
        }
        
        modify_buffer(debug_vertex_buffer, vertices.count * sizeof(VertexPCU), vertices.data);
        modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);
        draw(0, vertices.count);
    }
    set_primitive_type(GraphicsPrimitiveType::TRIANGLE);
#endif

    if(ui_visible) ui_render();

    swap_buffers();
}

void end_renderer() {
    release_texture(grid_texture);
    release_texture(sd_font_texture);
    end_graphics();
}