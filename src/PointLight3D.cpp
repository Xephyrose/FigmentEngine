#include "PointLight3D.h"

#include "AppState.h"
#include "SDL3/SDL_log.h"

PointLight3D::PointLight3D(AppState &appState) {
    Register(appState);
}

void PointLight3D::Register(AppState &appState) {
    appState.pointLights.push_back(this);
}

void PointLight3D::Unregister(AppState &appState) {
    if (const auto it = std::ranges::find(appState.pointLights, this); it != appState.pointLights.end()) {
        appState.pointLights.erase(it);
    }
    else {
        SDL_Log("Attempted to unregister an unregistered PointLight3D");
    }
}
