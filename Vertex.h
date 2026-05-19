#ifndef FIGMENTENGINE_VERTEX_H
#define FIGMENTENGINE_VERTEX_H
#include "Vector2.h"
#include "Vector3.h"

struct Vertex {
    Vector3 position;   // x, y, z
    Vector2 uv;         // u, v
    Vertex(float x = 0, float y = 0, float z = 0, float u = 0, float v = 0);
    Vertex(const Vector3 &position, const Vector2 &uv);
};


#endif //FIGMENTENGINE_VERTEX_H
