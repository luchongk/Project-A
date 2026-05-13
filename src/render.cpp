
#include "render.h"
#include "obj_loader.h"
//#include "ui.h"
#include "simple_draw.h"
#include "graphics_d3d11.h"
#include "d3dcompiler.h"
#include <cstdio>
#include "base/temp_allocator.h"

struct Framebuffer {
    ID3D11Texture2D* buffer;
    ID3D11RenderTargetView* render_target;
    ID3D11DepthStencilView* depth_stencil;
    ID3D11ShaderResourceView* shader_resource; // Optional
};

struct GraphicsBuffer {
    ID3D11Buffer* d3d;
};

struct Texture {
    //@Incomplete: What about ID3D11Texture3D or even ID3D11Texture3D?
    ID3D11Texture2D* d3d;
    ID3D11ShaderResourceView* shader_resource; // Optional
};

struct CompiledShader {
    union {
        ID3D11VertexShader*   vertex;
        ID3D11HullShader*     hull;
        ID3D11DomainShader*   domain;
        ID3D11GeometryShader* geometry;
        ID3D11PixelShader*    pixel;
        ID3D11ComputeShader*  compute;
    };
    ID3D10Blob* bytecode;
    ShaderStage stage;
};

// These will probably be loaded from somewhere
Model model_weird;
Model model_cube;
Model model_male;
Model model_female;

CompiledShader shader_vertex_basic;
CompiledShader shader_pixel_light;
CompiledShader shader_pixel_debug;

Texture* texture_grid;
Texture* texture_test;

MaterialBasic  MATERIAL_MISSING;
MaterialBasic  MATERIAL_GROUND;
MaterialBasic  MATERIAL_PLAYER;
MaterialBasic  MATERIAL_PLAYER2;
MaterialNoData MATERIAL_LIGHT;
//--------------------------------------------

Vector3 background = {0,0.02f,0.08f};

static OsWindow window;
static WindowGraphics* graphics;
Framebuffer onscreen_framebuffer;

static GraphicsBuffer static_vertex_buffer;
static GraphicsBuffer static_index_buffer;
static GraphicsBuffer debug_vertex_buffer;

static GraphicsBuffer global_uniform_buffer;
static GraphicsBuffer per_frame_uniform_buffer;
static GraphicsBuffer per_material_uniform_buffer;
static GraphicsBuffer per_object_uniform_buffer;

static GlobalUniforms    global_uniforms;
static PerFrameUniforms  per_frame_uniforms;
static PerObjectUniforms per_object_uniforms;

bool using_perspective = true;

void init_buffer(GraphicsBuffer* g, D3D11_BIND_FLAG type, D3D11_USAGE usage, uint size, void* data) {
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = size;
    buffer_desc.BindFlags = type;
    buffer_desc.Usage = usage;
    if(usage == D3D11_USAGE_DYNAMIC) buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA subresource_data{};
    D3D11_SUBRESOURCE_DATA* srd_pointer = nullptr;
    if(data) {
        subresource_data.pSysMem = data;
        srd_pointer = &subresource_data;
    }

    ID3D11Buffer* buffer;
    HRESULT error;
    error = d3d_device->CreateBuffer(&buffer_desc, srd_pointer, &buffer);

    g->d3d = buffer;
    return;
}

// This is for development only. Release builds should use precompiled shaders.
bool compile_shader(String path, CompiledShader* compiled, bool store_bytecode) {
    const char* target_version = nullptr;
    switch(compiled->stage) {
        case ShaderStage::VERTEX:   target_version = "vs_5_0"; break;
        case ShaderStage::HULL:     target_version = "hs_5_0"; break;
        case ShaderStage::DOMAIN:   target_version = "ds_5_0"; break;
        case ShaderStage::GEOMETRY: target_version = "gs_5_0"; break;
        case ShaderStage::PIXEL:    target_version = "ps_5_0"; break;
        case ShaderStage::COMPUTE:  target_version = "cs_5_0"; break;
        default: assert(false);
    }
    
    // ROW MAJOR vs COLUMN MAJOR STORAGE.
    //
    // mul(matrix, vector) with row major storage compiles down to 4 dot product instructions (1 dot product per matrix row) while the same operation with column major storage compiles to 1 MUL and 3 MADs.
    // The opposite is true if you are using a row vector convention (mul(vector, matrix)) in your shader code. Apparently, on modern hardware there's no difference in performance between the two sets of instructions, as mentioned
    // by Fabian Giesen in a comment on one of his posts: https://fgiesen.wordpress.com/2012/02/12/row-major-vs-column-major-row-vectors-vs-column-vectors/.
    //
    // So basically, it comes down to preference when choosing between D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR (the default) or D3DCOMPILE_PACK_MATRIX_ROW_MAJOR.
    // I'm going to use the latter just because I want to use column vector notation in my shader code, and translating that to 4 dot products in assembly reads better than the other option.

    char* c_path = to_cstring(path, temp_allocator);
    static wchar_t path_wide[256];
    mbstowcs(path_wide, c_path, 256);
    ID3D10Blob* errors = nullptr;
    HRESULT error;
    error = D3DCompileFromFile(path_wide, nullptr, nullptr, "main", target_version, D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &compiled->bytecode, &errors);
    if(errors) {
        printf((char*)errors->GetBufferPointer());
        errors->Release();
    }
    if(error != S_OK) return false;

    switch(compiled->stage) {
        case ShaderStage::VERTEX:   d3d_device->CreateVertexShader(compiled->bytecode->GetBufferPointer(),   compiled->bytecode->GetBufferSize(), nullptr, (ID3D11VertexShader**)&compiled->vertex);     break;
        case ShaderStage::HULL:     d3d_device->CreateHullShader(compiled->bytecode->GetBufferPointer(),     compiled->bytecode->GetBufferSize(), nullptr, (ID3D11HullShader**)&compiled->hull);         break;
        case ShaderStage::DOMAIN:   d3d_device->CreateDomainShader(compiled->bytecode->GetBufferPointer(),   compiled->bytecode->GetBufferSize(), nullptr, (ID3D11DomainShader**)&compiled->domain);     break;
        case ShaderStage::GEOMETRY: d3d_device->CreateGeometryShader(compiled->bytecode->GetBufferPointer(), compiled->bytecode->GetBufferSize(), nullptr, (ID3D11GeometryShader**)&compiled->geometry); break;
        case ShaderStage::PIXEL:    d3d_device->CreatePixelShader(compiled->bytecode->GetBufferPointer(),    compiled->bytecode->GetBufferSize(), nullptr, (ID3D11PixelShader**)&compiled->pixel);       break;
        case ShaderStage::COMPUTE:  d3d_device->CreateComputeShader(compiled->bytecode->GetBufferPointer(),  compiled->bytecode->GetBufferSize(), nullptr, (ID3D11ComputeShader**)&compiled->compute);   break;
        default: assert(false);
    }

    if(!store_bytecode) {
        compiled->bytecode->Release();
        compiled->bytecode = nullptr;
    }

    return true;
}

void init_texture_from_file(Texture* t, String path) {
    
}

void init_texture(Texture* t, int width, int height, int num_channels) {
    D3D11_TEXTURE2D_DESC texture_desc;
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    texture_desc.MiscFlags = 0;
    
    assert(num_channels >= 1 && num_channels <= 4);
    if(num_channels == 1) {
        texture_desc.Format = DXGI_FORMAT_R8_UNORM;
    }
    else if(num_channels == 2) {
        texture_desc.Format = DXGI_FORMAT_R8G8_UNORM;
    }
    else {
        texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    }

    ID3D11Texture2D *texture;
    HRESULT error = d3d_device->CreateTexture2D(&texture_desc, nullptr, &texture);

    ID3D11ShaderResourceView* shader_resource;
    error = d3d_device->CreateShaderResourceView(texture, nullptr, &shader_resource);

    t->d3d = texture;
    t->shader_resource = shader_resource;
}

void set_orthographic_projection(float width, float height, float z_near, float z_far) {
    global_uniforms.resolution = {width, height};
    global_uniforms.projection = orthographic(width, height, z_near, z_far);
    d3d_context->UpdateSubresource(global_uniform_buffer.d3d, 0, nullptr, &global_uniforms, 0, 0);
}

void set_perspective_projection(float width, float height, float z_near, float z_far) {
    global_uniforms.resolution = {width, height};
    global_uniforms.projection = perspective(width, height, 60.0f, z_near, z_far);
    d3d_context->UpdateSubresource(global_uniform_buffer.d3d, 0, nullptr, &global_uniforms, 0, 0);
}

void copy_model_to_buffers(Model* model, Array<VertexPNU>* vertices, Array<uint>* indices) {
    ForP(model->meshes) {
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

void init_renderer(OsWindow w) {
    window = w;
    graphics = init_window_graphics(window);

    //init_framebuffer(&onscreen_framebuffer, w, (int)window->size.x, (int)window->size.y);
    
    compile_shader("assets\\shaders\\basic_vertex.hlsl"_s, &shader_vertex_basic, false);
    compile_shader("assets\\shaders\\light_cube_pixel.hlsl"_s, &shader_pixel_light, false);
    compile_shader("assets\\shaders\\debug_pixel.hlsl"_s, &shader_pixel_debug, false);

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
    per_frame_uniforms.view_pos = main_camera->entity->position;
    per_frame_uniforms.view = look_to(main_camera->entity->position, main_camera->forward);
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
            per_object_uniforms.world = scale(Matrix::ident, Vector3{collider_size.x, collider_size.y, 0.1f});
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