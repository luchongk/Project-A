#ifndef GRAPHICS_H
#define GRAPHICS_H

enum class VertexFormat {
    PCU,
    PNU,
};

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

// Handle to a vertex, index or uniform buffer.
struct GraphicsBuffer;

struct Texture;

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

extern Texture* white_pixel;

bool init_graphics(HWND hwnd);

void end_graphics();

uint compile_shader(String vertex_path, String pixel_path, VertexFormat input_format);

void set_shader(uint id);

void set_fullscreen(bool fullscreen);

void set_framebuffer_size(int width, int height);

void bind_framebuffer();

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

void modify_buffer(GraphicsBuffer* buffer, uint size, void* data);

void set_vertex_buffer(GraphicsBuffer* buffer);

void set_index_buffer(GraphicsBuffer* buffer);

void set_uniform_buffer(UniformBufferSlot slot, GraphicsBuffer* buffer);

void set_primitive_type(GraphicsPrimitiveType type);

void draw(uint vertex_count);

void draw_indexed(uint index_count);

#endif