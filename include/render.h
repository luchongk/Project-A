#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "array.h"
#include "vector.h"

struct Material {
    String shader;
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float shininess;
};

struct Mesh {
    Array<Vec3>  vertices;
    Array<Vec3>  normals;
    Array<Vec2>  uvs;
    Array<uint>  indices;
};

void init_renderer();
void end_renderer();
void set_projection(int width, int height);

struct ::OSWindow;

void render(OSWindow* window);

extern Mesh weird_mesh;
extern Mesh cube_mesh;

extern uint pbr_shader;
extern uint light_shader;

#endif