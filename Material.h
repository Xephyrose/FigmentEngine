#ifndef FIGMENTENGINE_MATERIAL_H
#define FIGMENTENGINE_MATERIAL_H
#include <string>

#include "AppState.h"

struct Material {
    std::pmr::string name;
    std::string pipeline;

    virtual ~Material() = default;
    virtual void Bind(AppState* appState) const = 0;
    virtual void BindTextures(AppState* appState) const = 0;
};

#endif //FIGMENTENGINE_MATERIAL_H