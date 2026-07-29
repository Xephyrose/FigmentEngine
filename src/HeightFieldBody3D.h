#ifndef FIGMENTENGINE_HEIGHTFIELDBODY3D_H
#define FIGMENTENGINE_HEIGHTFIELDBODY3D_H
#include "PhysicsBody3D.h"
#include "SDL3/SDL_log.h"

struct HeightFieldBody3D : PhysicsBody3D {
    HeightFieldBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, float *heights, int countX, int countZ, b3Vec3 scale);
    HeightFieldBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, b3HeightFieldDef collider);
    HeightFieldBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, SDL_Surface* surface, int countX, int countZ, b3Vec3 scale);
};

inline float* CreateHeights(SDL_Surface *surface) {
    if (!surface)
    {
        SDL_Log("CreateHeights: surface is null");
        return nullptr;
    }

    SDL_Surface* rgba = surface;
    bool converted = false;

    if (surface->format != SDL_PIXELFORMAT_RGBA32) {
        rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        if (!rgba) {
            SDL_Log("CreateHeights: SDL_ConvertSurface failed: %s", SDL_GetError());
            return nullptr;
        }

        converted = true;
    }

    const auto* pixels = static_cast<const uint8_t*>(rgba->pixels);
    auto* points = new float[rgba->w * rgba->h];

    for (int z = 0; z < rgba->h; ++z)
    {
        const uint8_t* row = pixels + z * rgba->pitch;

        for (int x = 0; x < rgba->w; ++x)
        {
            const uint8_t* p = row + x * 4;

            const float r = p[0];

            points[z * rgba->w + x] = r / 255.0f;
        }
    }

    if (converted) SDL_DestroySurface(rgba);

    return points;
}

#endif //FIGMENTENGINE_HEIGHTFIELDBODY3D_H
