#ifndef RENDER_H
#define RENDER_H

#include "maths.h"
#include "graphics/d3d11/context.h"
#include "graphics/d3d11/types.h"

enum ConstantBufferSlot {
    CONSTANT_BUFFER_GLOBAL,
    CONSTANT_BUFFER_PER_FRAME,
    CONSTANT_BUFFER_PER_MATERIAL,
    CONSTANT_BUFFER_PER_OBJECT,
    CONSTANT_BUFFER_SLOT_COUNT,
};

struct ConstantBufferGlobal {
    Matrix projection;
    Vector2   resolution;
    float pad;
    float pad2;
};

struct ConstantBufferPerFrame {
    Matrix view;
    Vector3 view_pos;
    float pad;
    struct {
        Vector3 position;
        float pad;
        Vector3 ambient;
        float pad2;
        Vector3 diffuse;
        float pad3;
        Vector3 specular;
    } light;
    float time;
};

struct ConstantBufferPerObject {
    Matrix world;
};

enum VertexFormat {
    VERTEX_FORMAT_PCU,
    VERTEX_FORMAT_PNU,
    VERTEX_FORMAT_PCNU,
    VERTEX_FORMAT_COUNT,
};

struct VertexPCU {
    Vector3 position;
    Vector4 color;
    Vector2 uv;
};

struct VertexPNU {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

struct VertexPCNU {
    Vector3 position;
    Vector4 color;
    Vector3 normal;
    Vector2 uv;
};

// 6/5/2023: Material system seems good enough for now. One thing that I could change that would make the handling of textures in code a bit more safe is
// to make texture indices an enum class so that we can avoid using texture indices with any other materials than the one where the enum was defined.
// If I do that, the "safe" way to access textures would be through a pair of get and set functions defined for each struct instead of directly through vertex_textures/pixel_textures.

struct Material {
    Shader* vertex_shader;
    Shader* pixel_shader;
    Texture* vertex_textures[16];  // Count is the same as for shader.vertex_texture_slots
    Texture*  pixel_textures[16];  // Count is the same as for shader.pixel_texture_slots
    u64 constants_size;
    // Constants go after struct
};

inline void* get_material_constants(Material* material) {
    return (u8*)material + sizeof(Material);
}

struct MaterialNoData : Material {
    MaterialNoData() { constants_size = 0; }
};

struct MaterialBasic : Material {
    // Resource indices
    static const int _TEXTURE_INDEX = 0;
    
    MaterialBasic() { constants_size = sizeof(MaterialBasic) - sizeof(Material); }

    // Constants
    Vector3 ambient;
    float _pad1;
    Vector3 diffuse;
    float _pad2;
    Vector3 specular;
    float shininess;
};

struct Mesh {
    Array<Vector3> vertices;
    Array<Vector3> normals;
    Array<Vector2> uvs;
    Array<uint> indices;
    uint vertex_base;
    uint index_base;
};

struct Model {
    Array<Mesh> meshes;
};

void init_renderer();
void end_renderer();

void render();
void renderer_on_resize(int width, int height);

extern Model model_weird;
extern Model model_cube;
extern Model model_male;
extern Model model_female;
extern Texture texture_grid;

extern MaterialBasic  MATERIAL_MISSING;
extern MaterialBasic  MATERIAL_GROUND;
extern MaterialBasic  MATERIAL_PLAYER;
extern MaterialBasic  MATERIAL_PLAYER2;
extern MaterialNoData MATERIAL_LIGHT;

extern bool using_perspective;

#endif