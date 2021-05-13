#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "array.h"
#include "vector.h"

struct Mesh {
    Array<Vec3>  vertices;
    Array<Vec3>  normals;
    Array<Vec2>  uvs;
    Array<uint>     indices;
};

void init_renderer();
void end_renderer();

struct ::OSWindow;

void render(OSWindow* window);

#endif