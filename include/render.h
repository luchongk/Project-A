#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "array.h"
#include "vector.h"

// 6/5/2023: Material system seems good enough for now. One thing that I could change that would make the handling of textures in code a bit more safe is
// to make texture indices an enum class so that we can avoid using texture indices with any other materials than the one where the enum was defined.
// If I do that, the "safe" way to access textures would be through a pair of get and set functions defined for each struct instead of directly through vertex_textures/pixel_textures.

struct Material {
    ShaderProgram* shader;
    Texture* vertex_textures[16];  // Count is in shader.vertex_texture_slots
    Texture*  pixel_textures[16];  // Count is in shader.pixel_texture_slots
    u64 constants_size;
};

inline void* get_material_constants(Material* material) {
    return (u8*)material + sizeof(Material);
}

struct MaterialNoData : Material {
    MaterialNoData() { constants_size = sizeof(MaterialNoData) - sizeof(Material); }
};

struct MaterialBasic : Material {
    // Resource indices
    static const int _TEXTURE_INDEX = 0;
    
    MaterialBasic() { constants_size = sizeof(MaterialBasic) - sizeof(Material); }

    // Constants
    Vec3 ambient;
    float _pad1;
    Vec3 diffuse;
    float _pad2;
    Vec3 specular;
    float shininess;
};

struct Mesh {
    Array<Vec3> vertices;
    Array<Vec3> normals;
    Array<Vec2> uvs;
    Array<uint> indices;
    uint vertex_base;
    uint index_base;
};

struct Model {
    Array<Mesh> meshes;
};

void init_renderer();
void end_renderer();
void set_projection(int width, int height);

struct ::OSWindow;

void render(OSWindow* window);

extern Model model_weird;
extern Model model_cube;
extern Model model_male;
extern Model model_female;
extern Texture* texture_grid;

extern MaterialBasic  MATERIAL_GROUND;
extern MaterialNoData MATERIAL_LIGHT;

#endif