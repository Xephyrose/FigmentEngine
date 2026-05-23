#ifndef FIGMENTENGINE_MATERIAL_H
#define FIGMENTENGINE_MATERIAL_H
#include <string>

struct Material {
    std::pmr::string name;
    std::string shaderVertex;
    std::string shaderFragment;
};

#endif //FIGMENTENGINE_MATERIAL_H