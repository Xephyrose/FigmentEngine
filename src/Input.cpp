#include "Input.h"
#include <SDL3/SDL_keyboard.h>
#include <cstring>

#include "SDL3/SDL_log.h"

bool Input::prevState[SDL_SCANCODE_COUNT] = {false};
const bool* Input::currentState = SDL_GetKeyboardState(nullptr);

void Input::UpdateInputs() {
    memcpy(prevState, currentState, sizeof(prevState));
}

bool Input::IsPressed(const SDL_Scancode key) {
    return currentState && currentState[key];
}

bool Input::IsJustPressed(const SDL_Scancode key) {
    return currentState && currentState[key] && !prevState[key];
}

bool Input::IsJustReleased(const SDL_Scancode key) {
    return currentState && !currentState[key] && prevState[key];
}