
#include "render.h"
#include "obj_loader.h"
#include "matrix.h"
//#include "ui.h"
#include "new_ui.h"
#include "simple_draw.h"

struct GlobalUniforms {
    Matrix projection;
    Vec2   resolution;
    float pad;
    float pad2;
};

struct PerFrameUniforms {
    Matrix view;
    Vec3 view_pos;
    float pad;
    struct {
        Vec3 position;
        float pad;
        Vec3 ambient;
        float pad2;
        Vec3 diffuse;
        float pad3;
        Vec3 specular;
    } light;
    float time;
};

struct PerObjectUniforms {
    Matrix world;
};

Model model_weird;
Model model_cube;
Model model_male;
Model model_female;

ShaderProgram* shader_basic;
ShaderProgram* shader_light;
ShaderProgram* shader_debug;

Texture* texture_grid;
Texture* texture_test;

MaterialBasic  MATERIAL_MISSING;
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

bool using_perspective = true;

void set_orthographic_projection(float width, float height, float z_near, float z_far) {
    global_uniforms.resolution = window->size;
    global_uniforms.projection = orthographic(width, height, z_near, z_far);
    modify_buffer(global_uniform_buffer, sizeof(global_uniforms), &global_uniforms);
}

void set_perspective_projection(float width, float height, float z_near, float z_far) {
    global_uniforms.resolution = window->size;
    global_uniforms.projection = perspective(width, height, 60.0f, z_near, z_far);
    modify_buffer(global_uniform_buffer, sizeof(global_uniforms), &global_uniforms);
}

void copy_model_to_buffers(Model* model, Array<VertexPNU>* vertices, Array<uint>* indices) {
    For(model->meshes) {
        it->vertex_base = vertices->count;
        it->index_base  = indices->count;
        for(uint i = 0; i < it->vertices.count; i++) {
            array_add(vertices, {it->vertices[i], it->normals[i], it->uvs[i]});
        }

        for(uint i = 0; i < it->indices.count; i++) {
            array_add(indices, it->indices[i]);
        }
    }
}

void init_renderer(OSWindow* window) {
    init_graphics(window);
    
    onscreen_framebuffer = create_onscreen_framebuffer((int)window->size.x, (int)window->size.y);
    
    shader_basic = compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, "assets\\shaders\\basic_pixel.hlsl"_s,      VERTEX_FORMAT_PNU);
    shader_light = compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, "assets\\shaders\\light_cube_pixel.hlsl"_s, VERTEX_FORMAT_PNU);
    shader_debug = compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, "assets\\shaders\\debug_pixel.hlsl"_s,      VERTEX_FORMAT_PNU);

    texture_grid = create_texture_from_file("assets\\textures\\uv_grid_white.png"_s);

    // Material initialization
    {
        MATERIAL_MISSING.shader = shader_basic;
        MATERIAL_MISSING.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = white_pixel;
        MATERIAL_MISSING.ambient   = {1.0f, 0.0f, 1.0f};
        MATERIAL_MISSING.diffuse   = {1.0f, 0.0f, 1.0f};
        MATERIAL_MISSING.specular  = {1.0f, 0.0f, 1.0f};
        MATERIAL_MISSING.shininess = 32.0f;

        MATERIAL_GROUND.shader = shader_basic;
        MATERIAL_GROUND.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = white_pixel;
        MATERIAL_GROUND.ambient   = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.diffuse   = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.specular  = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.shininess = 32.0f;

        MATERIAL_PLAYER.shader = shader_basic;
        MATERIAL_PLAYER.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = white_pixel;
        MATERIAL_PLAYER.ambient   = {0.0f, 0.0f, 0.0f};
        MATERIAL_PLAYER.diffuse   = {1.0f, 1.0f, 1.0f};
        MATERIAL_PLAYER.specular  = {0.0f, 0.0f, 0.0f};
        MATERIAL_PLAYER.shininess = 1.0f;

        MATERIAL_PLAYER2 = MATERIAL_PLAYER;

        MATERIAL_LIGHT.shader = shader_light;
    }

    // Static mesh loading. @Speed: Doing multiple unnecessary buffer copies. Instead, pass vertices and indices to load_obj and add them directly there.
    {
        load_obj("assets\\models\\weirdchamp.obj"_s, &model_weird);
        load_obj("assets\\models\\cube.obj"_s,       &model_cube);
        load_obj("assets\\models\\Male.obj"_s,       &model_male);
        load_obj("assets\\models\\Female.obj"_s,     &model_female);

        Array<VertexPNU> vertices;
        vertices.allocator = linear_allocator(&temporary_storage);

        Array<uint> indices;
        indices.allocator = linear_allocator(&temporary_storage);

        copy_model_to_buffers(&model_weird,   &vertices, &indices);
        copy_model_to_buffers(&model_cube,    &vertices, &indices);
        copy_model_to_buffers(&model_male,    &vertices, &indices);
        copy_model_to_buffers(&model_female,  &vertices, &indices);
        
        static_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::STATIC, VERTEX_FORMAT_PNU, vertices.count, vertices.data);
        static_index_buffer  = create_index_buffer(GraphicsBufferUsage::STATIC, indices.count, indices.data);

        debug_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::DYNAMIC, VERTEX_FORMAT_PCU, 256);
    }
    
    global_uniform_buffer       = create_uniform_buffer(GraphicsBufferUsage::STATIC,  sizeof(GlobalUniforms));
    per_frame_uniform_buffer    = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, sizeof(PerFrameUniforms));
    per_material_uniform_buffer = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, 160);  //@Cleanup :Hardcoded size. Figure out a good one.
    per_object_uniform_buffer   = create_uniform_buffer(GraphicsBufferUsage::DYNAMIC, sizeof(PerObjectUniforms));

    set_uniform_buffer(UniformBufferSlot::PER_SETTINGS, global_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_FRAME,    per_frame_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_MATERIAL, per_material_uniform_buffer);
    set_uniform_buffer(UniformBufferSlot::PER_OBJECT,   per_object_uniform_buffer);

    set_primitive_type(GraphicsPrimitiveType::TRIANGLE);

    set_perspective_projection((float)window->size.x, (float)window->size.y);
    //ui_init();
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
        if(!e->model) continue;
        
        Material* material = e->material;
        if(!e->material) {
            material = &MATERIAL_MISSING;
        }

        if(!material->shader) {
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
        
        For(e->model->meshes) {
            draw_indexed(it->vertex_base, it->index_base, it->indices.count);
        }

#if 1
        //Render bounding box
        if(e->type == ENTITY_TYPE_Player) {
            auto collider_size = e->collider.box.max - e->collider.box.min;
            per_object_uniforms.world = scale(Matrix::ident, Vec3{collider_size.x, collider_size.y, 0.1f});
            per_object_uniforms.world = translate(per_object_uniforms.world, e->position);
            modify_buffer(per_object_uniform_buffer, sizeof(per_object_uniforms), &per_object_uniforms);
            set_primitive_type(GraphicsPrimitiveType::LINE);

            auto saved_shader = current_shader;
            set_shader(shader_debug);
            
            draw_indexed(model_cube.meshes[0].vertex_base, model_cube.meshes[0].index_base, model_cube.meshes[0].indices.count);
            
            set_primitive_type(GraphicsPrimitiveType::TRIANGLE);
            set_shader(saved_shader);
        }
#endif  
    }

#if 0
    // TODO: This is a good example of something we could do in a geometry shader, whenever we support those anyways...
    set_primitive_type(GraphicsPrimitiveType::LINE);
    set_vertex_buffer(debug_vertex_buffer);
    set_shader(shader_debug);
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

    ui_build();
    swap_buffers();
}

void end_renderer() {
    end_graphics();
}