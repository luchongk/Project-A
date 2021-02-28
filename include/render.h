#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "array.h"
#include "vector.h"

struct Mesh {
    Array<Vector3>  vertices;
    Array<Vector3>  normals;
    Array<Vector2>  uvs;
    Array<uint>     indices;
};

void init();

struct OSWindow;

void render(OSWindow* window);

#endif