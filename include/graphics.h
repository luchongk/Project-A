#ifndef GRAPHICS_H
#define GRAPHICS_H

ENUM(VertexFormat,
    PCU,
    PNU,
    COUNT,
)

struct VertexPCU {
    Vec3 position;
    Vec4 color;
    Vec2 uv;
};

struct VertexPNU {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
};

enum class GraphicsBufferUsage : u8 {
    //IMMUTABLE,
    STATIC,
    DYNAMIC
};

enum class GraphicsPrimitiveType : u8 {
    POINT,
    LINE,
    LINE_STRIP,
    TRIANGLE,
    TRIANGLE_STRIP
};

//The numeric value of these indicate register slots for the uniform buffers.
enum class UniformBufferSlot {
    PER_SETTINGS,
    PER_FRAME,
    PER_MATERIAL,
    PER_OBJECT
};

struct ShaderProgram {
    String vertex_name;
    String pixel_name;
    VertexFormat input_format;
    Array<uint> vertex_texture_slots;
    Array<uint> vertex_sampler_slots;
    Array<uint> pixel_texture_slots;
    Array<uint> pixel_sampler_slots;
};

// Handles

// Vertex, index or uniform buffer.
struct GraphicsBuffer;

struct Texture;
struct Framebuffer;

// API

extern ShaderProgram* current_shader;
extern Texture* white_pixel;

bool init_graphics(OSWindow* window);

void end_graphics();

ShaderProgram* compile_shader(String vertex_path, String pixel_path, VertexFormat input_format);

void set_shader(ShaderProgram* program);

Framebuffer* create_onscreen_framebuffer(int width, int height);
Framebuffer* create_offscreen_framebuffer(int width, int height);
void set_onscreen_framebuffer_size(int width, int height);
void set_offscreen_framebuffer_size(int width, int height);
Texture* get_framebuffer_texture(Framebuffer* framebuffer);
void bind_framebuffer(Framebuffer* framebuffer);

void swap_buffers();

void clear_color_buffer(float r, float g, float b);
void clear_depth_buffer();
void set_depth(bool on);
void set_blend(bool on);

Texture* create_texture_from_file(String path);
void set_texture(uint slot, Texture* texture);
void release_texture(Texture* texture);

GraphicsBuffer* create_vertex_buffer(GraphicsBufferUsage usage, VertexFormat format, uint count, void* data = nullptr);
GraphicsBuffer* create_index_buffer(GraphicsBufferUsage usage, uint count, void* data = nullptr);
GraphicsBuffer* create_uniform_buffer(GraphicsBufferUsage usage, uint size, void* data = nullptr);

void modify_buffer(GraphicsBuffer* buffer, u64 size, void* data);

void set_vertex_buffer(GraphicsBuffer* buffer);
void set_index_buffer(GraphicsBuffer* buffer);
void set_uniform_buffer(UniformBufferSlot slot, GraphicsBuffer* buffer);

void set_primitive_type(GraphicsPrimitiveType type);

void draw(uint vertex_base, uint vertex_count);
void draw_indexed(uint vertex_base, uint index_base, uint index_count);

#endif