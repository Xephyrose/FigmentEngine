#ifndef FIGMENTENGINE_INPUT_H
#define FIGMENTENGINE_INPUT_H
#include <SDL3/SDL_scancode.h>

struct Input {
    static void Update();
    static bool IsPressed(SDL_Scancode key);
    static bool IsJustPressed(SDL_Scancode key);
    static bool IsJustReleased(SDL_Scancode key);
    static bool prevState[SDL_SCANCODE_COUNT];
    static const bool* currentState;
};

#endif