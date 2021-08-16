#include <iostream>

#include "graphics.h"
#include "strings.h"
#include "memory.h"

#include "d3d11_4.h"
#include "d3dcompiler.h"
#include "dxgidebug.h"

struct ShaderProgram {
    ID3D11VertexShader* vertex_shader;
    ID3D11PixelShader*  pixel_shader;
    ID3D11InputLayout*  input_layout;
    VertexFormat input_format;
};

struct GraphicsBuffer {
    ID3D11Buffer* d3d;
    uint stride;
    GraphicsBufferUsage usage;
};

//struct Texture {};

static ID3D11Device5* device;
static ID3D11DeviceContext4* device_context;
static IDXGISwapChain2* swap_chain;
static ID3D11RenderTargetView* render_target;
static ID3D11DepthStencilView* depth_stencil;
static ID3D11DepthStencilState* depth_stencil_state_off;
static ID3D11BlendState1* blend_state_on;
static ID3D11RasterizerState2* rasterizer_state;
static ID3D11SamplerState* sampler_state;
static HANDLE frame_latency_waitable;

static const int MAX_SHADER_COUNT = 8;
static ShaderProgram shaders[MAX_SHADER_COUNT];
static int shader_count;

static const int MAX_BUFFER_COUNT = 16;
static GraphicsBuffer buffers[MAX_BUFFER_COUNT];
static int buffer_count;

//static ShaderProgram* current_shader = nullptr;
//static GraphicsBuffer* current_vertex_buffer = nullptr;

static uint get_format_stride(VertexFormat format) {
    switch(format) {
        case VertexFormat::PC:  return sizeof(VertexPC);
        case VertexFormat::PNU: return sizeof(VertexPNU);
        
        default: assert(!"UNREACHABLE");
    }

    return 0;
}

static void create_input_layout(VertexFormat format, ID3DBlob* bytecode, ShaderProgram* program) {
    switch(format) {
        case VertexFormat::PC: {
            D3D11_INPUT_ELEMENT_DESC attributes[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            HRESULT error;
            error = device->CreateInputLayout(attributes, 2, bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &program->input_layout);
        } break;

        case VertexFormat::PNU: {
            D3D11_INPUT_ELEMENT_DESC attributes[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"UV",       0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            HRESULT error;
            error = device->CreateInputLayout(attributes, 3, bytecode->GetBufferPointer(), bytecode->GetBufferSize(), &program->input_layout);
        } break;

        default: assert(false);
    }
}

static void create_color_and_depth_views() {
    HRESULT error;

    // Recreate render target view
    ID3D11Texture2D* pBuffer;
    error = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBuffer);

    error = device->CreateRenderTargetView(pBuffer, nullptr, &render_target);
    
    pBuffer->Release();

    // Recreate depth stencil view
    uint width, height;
    swap_chain->GetSourceSize(&width, &height);

    ID3D11Texture2D* depth_stencil_texture;
    D3D11_TEXTURE2D_DESC texture_desc;
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = 0;
    error = device->CreateTexture2D(&texture_desc, nullptr, &depth_stencil_texture);

    // Create the depth stencil view
    error = device->CreateDepthStencilView(depth_stencil_texture, nullptr, &depth_stencil);

    depth_stencil_texture->Release();
}

bool init_graphics(HWND hwnd) {
    ID3D11Device* dev11;
    ID3D11DeviceContext* devcon11;

    HRESULT error;
    error = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &dev11,
        nullptr,
        &devcon11);

    error = dev11->QueryInterface(__uuidof(ID3D11Device5), (void**)&device);
    error = devcon11->QueryInterface(__uuidof(ID3D11DeviceContext4), (void**)&device_context);

    dev11->Release();
    devcon11->Release();

    IDXGIDevice4* dxgi_device;
    error = device->QueryInterface(__uuidof(IDXGIDevice4), (void**)&dxgi_device);

    IDXGIAdapter* dxgiAdapter;
    error = dxgi_device->GetAdapter(&dxgiAdapter);

    error = dxgi_device->SetMaximumFrameLatency(1);

    dxgi_device->Release();

    IDXGIFactory2* dxgiFactory;
    error = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);

    dxgiAdapter->Release();

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // We are using this to render to the display
    swap_chain_desc.BufferCount = 2;                                  // Double buffering
    swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;              // the most common swap chain format
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       // the recommended flip mode
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;                           // disable anti-aliasing
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT; //DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT

    IDXGISwapChain1* sc1;
    error = dxgiFactory->CreateSwapChainForHwnd(device, hwnd, &swap_chain_desc, nullptr, nullptr, &sc1);

    error = sc1->QueryInterface(__uuidof(IDXGISwapChain2), (void**)&swap_chain);
    sc1->Release();

    error = dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);

    dxgiFactory->Release();

    frame_latency_waitable = swap_chain->GetFrameLatencyWaitableObject();

    create_color_and_depth_views();

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
    
    device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state_off);

    D3D11_RENDER_TARGET_BLEND_DESC1 rt_blend_desc;
    rt_blend_desc.BlendEnable           = true;
    rt_blend_desc.LogicOpEnable         = false;
    rt_blend_desc.SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    rt_blend_desc.DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    rt_blend_desc.BlendOp               = D3D11_BLEND_OP_ADD;
    rt_blend_desc.SrcBlendAlpha         = D3D11_BLEND_ONE;
    rt_blend_desc.DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
    rt_blend_desc.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    rt_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC1 blend_desc;
    blend_desc.AlphaToCoverageEnable  = false;
    blend_desc.IndependentBlendEnable = false;
    blend_desc.RenderTarget[0] = rt_blend_desc;
    device->CreateBlendState1(&blend_desc, &blend_state_on);
    
    CD3D11_RASTERIZER_DESC2 rasterizer_desc{D3D11_DEFAULT};
    rasterizer_desc.FrontCounterClockwise = true;
    device->CreateRasterizerState2(&rasterizer_desc, &rasterizer_state);
    device_context->RSSetState(rasterizer_state);

    CD3D11_SAMPLER_DESC sampler_desc{D3D11_DEFAULT};
    device->CreateSamplerState(&sampler_desc, &sampler_state);
    device_context->PSSetSamplers(0, 1, &sampler_state);

    // Set up the viewport.
    D3D11_VIEWPORT vp;
    vp.Width = 1600;
    vp.Height = 800;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    device_context->RSSetViewports( 1, &vp );

    return true;
}

void end_graphics() {
    //HRESULT error;
    //error = swap_chain->SetFullscreenState(false, nullptr);

    for(int i = 0; i < buffer_count; i++) {
        buffers[i].d3d->Release();
    }

    for(int i = 0; i < shader_count; i++) {
        shaders[i].input_layout->Release();
        shaders[i].pixel_shader->Release();
        shaders[i].vertex_shader->Release();
    }

    sampler_state->Release();
    rasterizer_state->Release();
    depth_stencil_state_off->Release();
    blend_state_on->Release();
    depth_stencil->Release();
    render_target->Release();
    swap_chain->Release();
    device_context->Release();
    device->Release();

    /*ID3D11Debug* debug;
    device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
    debug->ReportLiveDeviceObjects((D3D11_RLDO_FLAGS)7);*/
}

void wait_for_vblank() {
    WaitForSingleObjectEx(frame_latency_waitable, 1000, true);
}

//@Cleanup: We could use ID3D11ShaderReflection to get rid of the input_format parameter here, and maybe some other stuff too...
uint compile_shader(String vertex_path, String pixel_path, VertexFormat input_format) {
    ID3D10Blob* bytecode;

    assert(shader_count < MAX_SHADER_COUNT);
    ShaderProgram* program = &shaders[shader_count];
    program->input_format = input_format;

    wchar_t path_wide[100];

    //VERTEX SHADER
    mbstowcs(path_wide, (const char*)vertex_path.data, 100);

    HRESULT error;
    ID3D10Blob* errors;
    error = D3DCompileFromFile(path_wide, nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &bytecode, &errors);
    if (error)
    {
        auto error_message = (char*)errors->GetBufferPointer();
        std::cout << error_message << std::endl;
        errors->Release();
        return 0;
    }

    error = device->CreateVertexShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, &program->vertex_shader);

    create_input_layout(input_format, bytecode, program);

    //PIXEL SHADER
    mbstowcs(path_wide, (const char*)pixel_path.data, 100);

    error = D3DCompileFromFile(path_wide, nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &bytecode, &errors);
    if (error)
    {
        auto error_message = (char*)errors->GetBufferPointer();
        std::cout << error_message << std::endl;
        errors->Release();
        return 0;
    }

    error = device->CreatePixelShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, &program->pixel_shader);

    bytecode->Release();

    uint id = shader_count;
    shader_count++;

    return id;
}

void set_shader(uint id) {
    ShaderProgram* program = &shaders[id];

    device_context->VSSetShader(program->vertex_shader, nullptr, 0);
    device_context->PSSetShader(program->pixel_shader, nullptr, 0);

    device_context->IASetInputLayout(program->input_layout);
}

void set_fullscreen(bool fullscreen) {
    HRESULT error;
    error = swap_chain->SetFullscreenState(fullscreen, nullptr);
}

void adjust_size(int width, int height) {
    device_context->OMSetRenderTargets(0, nullptr, nullptr);

    render_target->Release();
    depth_stencil->Release();

    HRESULT error;
    // Preserve the existing buffer count and format.
    error = swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT/* DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT*/);
                        
    create_color_and_depth_views();

    // Set up the viewport.
    D3D11_VIEWPORT vp;
    vp.Width  = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    device_context->RSSetViewports(1, &vp);
}

// We only have 1 framebuffer at the moment so we dont take any parameters
void bind_framebuffer() {
    device_context->OMSetRenderTargets(1, &render_target, depth_stencil);
}

void swap_buffers() {
    HRESULT error;
    error = swap_chain->Present(1, 0/*DXGI_PRESENT_ALLOW_TEARING*/);
}

void clear_color_buffer(float r, float g, float b) {
    float color[4] = {r, g, b, 1.0f};
    device_context->ClearRenderTargetView(render_target, color);
}

void clear_depth_buffer() {
    device_context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void set_depth(bool on) {
    if(on) device_context->OMSetDepthStencilState(nullptr, 1);
    else   device_context->OMSetDepthStencilState(depth_stencil_state_off, 1);
}

void set_blend(bool on) {
    if(on) device_context->OMSetBlendState(blend_state_on, nullptr, 0xFFFFFFFF);
    else   device_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

Texture* create_texture(String path) {
    stbi_set_flip_vertically_on_load(true);  
    
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *image_data = stbi_load((const char*)path.data, &width, &height, &nrChannels, 0);
    if (!image_data) return nullptr;

    D3D11_TEXTURE2D_DESC texture_desc;
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.MipLevels = 0;
    texture_desc.ArraySize = 1;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    /*D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = image_data;
    initData.SysMemPitch = width * nrChannels;*/

    ID3D11Texture2D *texture;
    HRESULT error;
    error = device->CreateTexture2D(&texture_desc, nullptr, &texture);

    ID3D11ShaderResourceView* texture_view;
    error = device->CreateShaderResourceView(texture, nullptr, &texture_view);
    
    device_context->UpdateSubresource(texture, 0, 0, image_data, width * nrChannels, 0);

    stbi_image_free(image_data);
    texture->Release();

    device_context->GenerateMips(texture_view);

    return (Texture*)texture_view;
}

void set_texture(uint slot, Texture* texture) {
    device_context->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&texture);
}

void release_texture(Texture* texture) {
    ((ID3D11ShaderResourceView*)texture)->Release();
}

static GraphicsBuffer* create_buffer(D3D11_BIND_FLAG type, GraphicsBufferUsage usage, uint size, void* data) {
    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = size;
    buffer_desc.BindFlags = type;

    if(usage == GraphicsBufferUsage::STATIC) {
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    }
    else {
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }

    D3D11_SUBRESOURCE_DATA* subresource_data = nullptr;
    if(data) {
        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = data;
        subresource_data = &srd;
    }

    assert(buffer_count < MAX_BUFFER_COUNT);
    GraphicsBuffer* buffer = &buffers[buffer_count];
    buffer->usage = usage;

    HRESULT error;
    error = device->CreateBuffer(&buffer_desc, subresource_data, &buffer->d3d);

    buffer_count++;

    return buffer;
}

GraphicsBuffer* create_vertex_buffer(GraphicsBufferUsage usage, VertexFormat format, uint count, void* data) {
    uint stride = get_format_stride(format);
    GraphicsBuffer* buffer = create_buffer(D3D11_BIND_VERTEX_BUFFER, usage, count * stride, data);
    buffer->stride = stride;

    return buffer;
}

GraphicsBuffer* create_index_buffer(GraphicsBufferUsage usage, uint count, void* data) {
    return create_buffer(D3D11_BIND_INDEX_BUFFER, usage, sizeof(uint) * count, data);
}

GraphicsBuffer* create_uniform_buffer(GraphicsBufferUsage usage, uint size, void* data) {
    return create_buffer(D3D11_BIND_CONSTANT_BUFFER, usage, size, data);
}

void set_vertex_buffer(GraphicsBuffer* buffer) {
    UINT stride = buffer->stride;
    UINT offset = 0;
    device_context->IASetVertexBuffers(0, 1, &buffer->d3d, &stride, &offset);
}

void set_index_buffer(GraphicsBuffer* buffer) {
    device_context->IASetIndexBuffer(buffer->d3d, DXGI_FORMAT_R32_UINT, 0);
}

void set_uniform_buffer(UniformBufferSlot slot, GraphicsBuffer* buffer) {
    device_context->VSSetConstantBuffers((uint)slot, 1, &buffer->d3d);
    device_context->PSSetConstantBuffers((uint)slot, 1, &buffer->d3d);
}

void modify_buffer(GraphicsBuffer* buffer, uint size, void* data) {
    // @Cleanup: GraphicsBuffer should probably be separated into VertexBuffer, IndexBuffer, ConsantBuffer, etc.
    // We almost never treat them the same and there are cases like this where we can't assume there is a stride
    // (constant buffers don't have stride), to automatically get the size of a vertex buffer.
    if(buffer->usage == GraphicsBufferUsage::DYNAMIC) {
        HRESULT error;
        D3D11_MAPPED_SUBRESOURCE resource;
        error = device_context->Map(buffer->d3d, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        memcpy(resource.pData, data, size);
        device_context->Unmap(buffer->d3d, 0);
    }
    else if(buffer->usage == GraphicsBufferUsage::STATIC) {
        device_context->UpdateSubresource(buffer->d3d, 0, nullptr, data, 0, 0);
    }
}

void set_primitive_type(GraphicsPrimitiveType type) {
    D3D11_PRIMITIVE_TOPOLOGY d3d_type = (D3D11_PRIMITIVE_TOPOLOGY)((int)type + 1);

    device_context->IASetPrimitiveTopology(d3d_type);
}

void draw(uint vertex_count) {
    device_context->Draw(vertex_count, 0);
}

void draw_indexed(uint index_count) {
    device_context->DrawIndexed(index_count, 0, 0);
}