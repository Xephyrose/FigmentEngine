#ifndef FIGMENTENGINE_HEIGHTFIELDBODY3D_H
#define FIGMENTENGINE_HEIGHTFIELDBODY3D_H
#include "PhysicsBody3D.h"

struct HeightFieldBody3D : PhysicsBody3D {
    HeightFieldBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, float *heights, int countX, int countZ, b3Vec3 scale);
    HeightFieldBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, b3HeightFieldDef collider);
};

// currently assumes SDL_Surface is format SDL_PIXELFORMAT_INDEX8
inline float* CreateHeights(const SDL_Surface *surface) {
    const auto* pixels = static_cast<const uint8_t*>(surface->pixels);
    auto* points = new float[surface->w * surface->h];

    for (int z = 0; z < surface->h; ++z)
    {
        for (int x = 0; x < surface->w; ++x)
        {
            points[z * surface->w + x] = static_cast<float>(pixels[z * surface->pitch + x]) / 255.0f;
        }
    }

    return points;
}

#endif //FIGMENTENGINE_HEIGHTFIELDBODY3D_H
