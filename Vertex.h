#ifndef FIGMENTENGINE_VERTEX_H
#define FIGMENTENGINE_VERTEX_H

#include "Vector3.h"
#include "Vector2.h"

struct Vertex {
    float x, y, z;
    Vertex(float x, float y, float z);

    Vector3 verts;
    Vector2 uv;
};


#endif //FIGMENTENGINE_VERTEX_H
