
#include <cstdio>

#include "render.h"
#include "base/temp_allocator.h"
#include "graphics/d3d11/utils.h"
#include "general.h"
#include "entities.h"
#include "obj_loader.h"
#include "simple_draw.h"

// These will probably be loaded from somewhere
Model model_weird;
Model model_cube;
Model model_male;
Model model_female;

Shader shader_vertex_basic;
Shader shader_pixel_basic;
Shader shader_pixel_light;
Shader shader_pixel_debug;

Texture texture_grid;
Texture white_pixel;

MaterialBasic  MATERIAL_MISSING;
MaterialBasic  MATERIAL_GROUND;
MaterialBasic  MATERIAL_PLAYER;
MaterialBasic  MATERIAL_PLAYER2;
MaterialNoData MATERIAL_LIGHT;
//--------------------------------------------

Vector3 background = {0,0.02f,0.08f};

static WindowGraphics* graphics;

static ID3D11DepthStencilState* depth_stencil_state_off;
static ID3D11BlendState1* blend_state_on;
static ID3D11SamplerState* sampler_state;
static ID3D11RasterizerState2* rasterizer_state;

ID3D11DepthStencilView* depth_stencil_view;

static GraphicsBuffer static_vertex_buffer;
static GraphicsBuffer static_index_buffer;
static GraphicsBuffer debug_vertex_buffer;

static GraphicsBuffer constant_buffer_global;
static GraphicsBuffer constant_buffer_per_frame;
static GraphicsBuffer constant_buffer_per_material;
static GraphicsBuffer constant_buffer_per_object;

static ConstantBufferGlobal    constants_global;
static ConstantBufferPerFrame  constants_per_frame;
static ConstantBufferPerObject constants_per_object;

static ID3D11InputLayout* input_layouts[VERTEX_FORMAT_COUNT];

bool using_perspective = true;

static void create_input_layout(VertexFormat format, ID3DBlob* bytecode) {
    switch(format) {
        case VERTEX_FORMAT_PCU: {
            D3D11_INPUT_ELEMENT_DESC attributes[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV",       0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            HRESULT error;
            error = d3d_device->CreateInputLayout(attributes, 3, bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &input_layouts[VERTEX_FORMAT_PCU]);
            
            break;
        }

        case VERTEX_FORMAT_PNU: {
            D3D11_INPUT_ELEMENT_DESC attributes[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV",       0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            HRESULT error;
            error = d3d_device->CreateInputLayout(attributes, 3, bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &input_layouts[VERTEX_FORMAT_PNU]);

            break;
        }

        default: assert(false);
    }
}

void update_and_bind_constant_buffer(GraphicsBuffer* buffer, void* data, u64 size, int slot) {
    copy_to_buffer(buffer->d3d, data, size);
    d3d_context->VSSetConstantBuffers(slot, 1, &buffer->d3d);
    d3d_context->PSSetConstantBuffers(slot, 1, &buffer->d3d);
}

static void set_orthographic_projection(float width, float height, float z_near = 0.1f, float z_far = 100.0f) {
    constants_global.resolution = {width, height};
    constants_global.projection = orthographic(width, height, z_near, z_far);

    update_and_bind_constant_buffer(&constant_buffer_global, &constants_global, sizeof(constants_global), CONSTANT_BUFFER_GLOBAL);
}

static void set_perspective_projection(float width, float height, float z_near = 0.1f, float z_far = 100.0f) {
    constants_global.resolution = {width, height};
    constants_global.projection = perspective(width, height, 60.0f, z_near, z_far);

    update_and_bind_constant_buffer(&constant_buffer_global, &constants_global, sizeof(constants_global), CONSTANT_BUFFER_GLOBAL);
}

void copy_model_to_buffers(Model* model, Array<VertexPNU>* vertices, Array<uint>* indices) {
    ForP(model->meshes) {
        it->vertex_base = (u32)vertices->count;
        it->index_base  = (u32)indices->count;
        for(uint i = 0; i < it->vertices.count; i++) {
            array_add(vertices, {it->vertices[i], it->normals[i], it->uvs[i]});
        }

        for(uint i = 0; i < it->indices.count; i++) {
            array_add(indices, it->indices[i]);
        }
    }
}

void renderer_on_resize(int width, int height) {    
    d3d_context->OMSetRenderTargets(0, nullptr, nullptr);
    
    resize_buffers(the_window, width, height);
    if(depth_stencil_view) depth_stencil_view->Release();
    depth_stencil_view = create_depth_stencil_view(width, height);

    auto size = Vector2{(float)width, (float)height};
    if(using_perspective) {
        set_perspective_projection(size.x, size.y);
    } else {
        set_orthographic_projection(20, 20.0f * size.y / size.x);
    }

    set_viewport(width, height);
}

void init_renderer() {
    graphics = init_window_graphics(the_window);

    // PIPELINE STATE
    {
        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
        depth_stencil_desc.DepthEnable = false;
        depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        //depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
        
        depth_stencil_desc.StencilEnable = false;
        /*depth_stencil_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        depth_stencil_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        // Stencil operations if pixel is front-facing
        depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        // Stencil operations if pixel is back-facing
        depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;*/
        
        d3d_device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state_off);

        D3D11_RENDER_TARGET_BLEND_DESC1 rt_blend_desc;
        rt_blend_desc.BlendEnable           = true;
        rt_blend_desc.LogicOpEnable         = false;
        rt_blend_desc.SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        rt_blend_desc.DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        rt_blend_desc.BlendOp               = D3D11_BLEND_OP_ADD;
        rt_blend_desc.SrcBlendAlpha         = D3D11_BLEND_ONE;
        rt_blend_desc.DestBlendAlpha        = D3D11_BLEND_ONE;
        rt_blend_desc.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        rt_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        D3D11_BLEND_DESC1 blend_desc;
        blend_desc.AlphaToCoverageEnable  = false;
        blend_desc.IndependentBlendEnable = false;
        blend_desc.RenderTarget[0] = rt_blend_desc;
        d3d_device->CreateBlendState1(&blend_desc, &blend_state_on);

        CD3D11_SAMPLER_DESC sampler_desc{D3D11_DEFAULT};
        d3d_device->CreateSamplerState(&sampler_desc, &sampler_state);

        CD3D11_RASTERIZER_DESC2 rasterizer_desc{D3D11_DEFAULT};
        rasterizer_desc.FrontCounterClockwise = true;
        d3d_device->CreateRasterizerState2(&rasterizer_desc, &rasterizer_state);
        //d3d_context->RSSetState(rasterizer_state);
    }
    
    shader_vertex_basic.stage = ShaderStage::VERTEX;
    shader_vertex_basic.vertex = (ID3D11VertexShader*)compile_shader(&shader_vertex_basic.bytecode, "assets\\shaders\\basic_vertex.hlsl"_s, "vs_5_0"_s);
    
    create_input_layout(VertexFormat::VERTEX_FORMAT_PNU, shader_vertex_basic.bytecode);
    
    shader_pixel_basic.stage = ShaderStage::PIXEL;
    shader_pixel_basic.pixel = (ID3D11PixelShader*)compile_shader(nullptr, "assets\\shaders\\basic_pixel.hlsl"_s, "ps_5_0"_s);
    
    shader_pixel_light.stage = ShaderStage::PIXEL;
    shader_pixel_light.pixel = (ID3D11PixelShader*)compile_shader(nullptr, "assets\\shaders\\light_cube_pixel.hlsl"_s, "ps_5_0"_s);
    
    shader_pixel_debug.stage = ShaderStage::PIXEL;
    shader_pixel_debug.pixel = (ID3D11PixelShader*)compile_shader(nullptr, "assets\\shaders\\debug_pixel.hlsl"_s, "ps_5_0"_s);

    texture_grid.d3d = load_texture_from_file(&texture_grid.shader_resource, "assets\\textures\\uv_grid_white.png"_s, true);

    white_pixel.d3d = create_texture(&white_pixel.shader_resource, 1, 1, 4);
    u8 white[4] = {255, 255, 255, 255};
    d3d_context->UpdateSubresource(white_pixel.d3d, 0, nullptr, white, 4, 0);

    // Material initialization
    {
        MATERIAL_MISSING.vertex_shader = &shader_vertex_basic;
        MATERIAL_MISSING.pixel_shader  = &shader_pixel_basic;
        MATERIAL_MISSING.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = &white_pixel;
        MATERIAL_MISSING.ambient   = {1.0f, 0.0f, 1.0f};
        MATERIAL_MISSING.diffuse   = {1.0f, 0.0f, 1.0f};
        MATERIAL_MISSING.specular  = {1.0f, 0.0f, 1.0f};
        MATERIAL_MISSING.shininess = 32.0f;

        MATERIAL_GROUND.vertex_shader = &shader_vertex_basic;
        MATERIAL_GROUND.pixel_shader  = &shader_pixel_basic;
        MATERIAL_GROUND.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = &white_pixel;
        MATERIAL_GROUND.ambient   = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.diffuse   = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.specular  = {1.0f, 1.0f, 1.0f};
        MATERIAL_GROUND.shininess = 32.0f;

        MATERIAL_PLAYER.vertex_shader = &shader_vertex_basic;
        MATERIAL_PLAYER.pixel_shader  = &shader_pixel_basic;
        MATERIAL_PLAYER.pixel_textures[MaterialBasic::_TEXTURE_INDEX] = &white_pixel;
        MATERIAL_PLAYER.ambient   = {0.0f, 0.0f, 0.0f};
        MATERIAL_PLAYER.diffuse   = {1.0f, 1.0f, 1.0f};
        MATERIAL_PLAYER.specular  = {0.0f, 0.0f, 0.0f};
        MATERIAL_PLAYER.shininess = 1.0f;

        MATERIAL_PLAYER2 = MATERIAL_PLAYER;

        MATERIAL_LIGHT.vertex_shader = &shader_vertex_basic;
        MATERIAL_LIGHT.pixel_shader  = &shader_pixel_light;
    }

    // Static mesh loading. @Speed: Doing multiple unnecessary buffer copies. Instead, pass vertices and indices to load_obj and add them directly there.
    {
        load_obj("assets\\models\\weirdchamp.obj"_s, &model_weird);
        load_obj("assets\\models\\cube.obj"_s,       &model_cube);
        load_obj("assets\\models\\Male.obj"_s,       &model_male);
        load_obj("assets\\models\\Female.obj"_s,     &model_female);

        Array<VertexPNU> vertices;
        vertices.allocator = temp_allocator;

        Array<uint> indices;
        indices.allocator = temp_allocator;

        copy_model_to_buffers(&model_weird,   &vertices, &indices);
        copy_model_to_buffers(&model_cube,    &vertices, &indices);
        copy_model_to_buffers(&model_male,    &vertices, &indices);
        copy_model_to_buffers(&model_female,  &vertices, &indices);
        
        static_vertex_buffer.d3d = create_buffer(D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, (uint)(sizeof(VertexPNU) * vertices.count), vertices.data);
        static_index_buffer.d3d  = create_buffer(D3D11_BIND_INDEX_BUFFER,  D3D11_USAGE_DEFAULT, (uint)(sizeof(uint) * indices.count), indices.data);
        debug_vertex_buffer.d3d  = create_buffer(D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, (uint)(sizeof(VertexPCU) * 256));
    }
    
    constant_buffer_global.d3d       = create_buffer(D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, sizeof(ConstantBufferGlobal), &constants_global);
    constant_buffer_per_frame.d3d    = create_buffer(D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, sizeof(ConstantBufferPerFrame), &constants_per_frame);
    constant_buffer_per_material.d3d = create_buffer(D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, 160);
    constant_buffer_per_object.d3d   = create_buffer(D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, sizeof(ConstantBufferPerObject), &constants_per_object);

    auto window_size = get_window_size(the_window);
    renderer_on_resize(window_size.x, window_size.y);
}

Shader* current_vertex_shader;
Shader* current_pixel_shader;

void render() {
    d3d_context->RSSetState(rasterizer_state);
    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    d3d_context->OMSetRenderTargets(1, &graphics->render_target, depth_stencil_view);
    
    d3d_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    d3d_context->OMSetDepthStencilState(nullptr, 0);
    
    d3d_context->ClearRenderTargetView(graphics->render_target, background.elems);
    d3d_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    update_and_bind_constant_buffer(&constant_buffer_global, &constants_global, sizeof(constants_global), CONSTANT_BUFFER_GLOBAL);

    constants_per_frame.light.position = light->entity->position;
    constants_per_frame.light.diffuse  = light->diffuse;
    constants_per_frame.light.ambient  = light->ambient;
    constants_per_frame.light.specular = light->specular;
    constants_per_frame.time = my_time.since_start;
    constants_per_frame.view_pos = main_camera->entity->position;
    constants_per_frame.view = look_to(main_camera->entity->position, main_camera->forward);
    update_and_bind_constant_buffer(&constant_buffer_per_frame, &constants_per_frame, sizeof(constants_per_frame), CONSTANT_BUFFER_PER_FRAME);

    uint stride = sizeof(VertexPNU);
    uint offsets = 0;
    d3d_context->IASetVertexBuffers(0, 1, &static_vertex_buffer.d3d, &stride, &offsets);
    d3d_context->IASetIndexBuffer(static_index_buffer.d3d, DXGI_FORMAT_R32_UINT, 0);

    
    current_vertex_shader = nullptr;
    current_pixel_shader = nullptr;
    for(int i = 0; i < entity_count; i++) {
        Entity* e = &entities[i];
        if(!e->model) continue;
        
        Material* material = e->material;
        if(!e->material) {
            material = &MATERIAL_MISSING;
        }

        if(!material->vertex_shader || !material->pixel_shader) {
            printf("Tried to draw an entity with null shader\n");
            continue;
        }

        d3d_context->IASetInputLayout(input_layouts[VERTEX_FORMAT_PNU]);

        //@Speed: Sort by shader/material instead!
        if(current_vertex_shader != material->vertex_shader) {
            d3d_context->VSSetShader(material->vertex_shader->vertex, nullptr, 0);
            current_vertex_shader = material->vertex_shader;
        }
        if(current_pixel_shader != material->pixel_shader) {
            d3d_context->PSSetShader(material->pixel_shader->pixel, nullptr, 0);
            current_pixel_shader = material->pixel_shader;
        }
        d3d_context->PSSetSamplers(0, 1, &sampler_state);
        
        for(uint j = 0; j < 16; j++) {
            if(material->pixel_textures[j]) {
                d3d_context->PSSetShaderResources(j, 1, &material->pixel_textures[j]->shader_resource);
            }
        }

        if(material->constants_size > 0) {
            assert(material->constants_size < 160); // See :Hardcoded
            void* constants = get_material_constants(material);
            update_and_bind_constant_buffer(&constant_buffer_per_material, constants, material->constants_size, CONSTANT_BUFFER_PER_MATERIAL);
        }
        
        constants_per_object.world = get_world_matrix(e);
        update_and_bind_constant_buffer(&constant_buffer_per_object, &constants_per_object, sizeof(constants_per_object), CONSTANT_BUFFER_PER_OBJECT);
        
        For(e->model->meshes) {
            d3d_context->DrawIndexed((uint)it.indices.count, it.index_base, it.vertex_base);
        }

#if 1
        //Render bounding box
        if(e->type == ENTITY_TYPE_Player) {
            auto collider_size = e->collider.box.max - e->collider.box.min;
            constants_per_object.world = scale(Matrix::ident, Vector3{collider_size.x, collider_size.y, 0.1f});
            constants_per_object.world = translate(constants_per_object.world, e->position);
            update_and_bind_constant_buffer(&constant_buffer_per_object, &constants_per_object, sizeof(constants_per_object), CONSTANT_BUFFER_PER_OBJECT);
            d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

            auto saved_pixel_shader = current_pixel_shader;
            d3d_context->PSSetShader(shader_pixel_debug.pixel, nullptr, 0);
            
            auto mesh = &model_cube.meshes[0];
            d3d_context->DrawIndexed((uint)mesh->indices.count, mesh->index_base, mesh->vertex_base);
            
            d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            d3d_context->PSSetShader(saved_pixel_shader->pixel, nullptr, 0);
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
}

void end_renderer() {
    shader_vertex_basic.vertex->Release();
    shader_vertex_basic.bytecode->Release();
    shader_pixel_basic.pixel->Release();
    shader_pixel_light.pixel->Release();
    shader_pixel_debug.pixel->Release();

    texture_grid.d3d->Release();
    texture_grid.shader_resource->Release();
    white_pixel.d3d->Release();
    white_pixel.shader_resource->Release();

    depth_stencil_state_off->Release();
    blend_state_on->Release();
    sampler_state->Release();
    rasterizer_state->Release();

    if(depth_stencil_view) depth_stencil_view->Release();

    static_vertex_buffer.d3d->Release();
    static_index_buffer.d3d->Release();
    debug_vertex_buffer.d3d->Release();

    constant_buffer_global.d3d->Release();
    constant_buffer_per_frame.d3d->Release();
    constant_buffer_per_material.d3d->Release();
    constant_buffer_per_object.d3d->Release();

    for(int i = 0; i < VERTEX_FORMAT_COUNT; i++) {
        if(input_layouts[i]) input_layouts[i]->Release();
    }

    sd_free();
    
    end_graphics();
}