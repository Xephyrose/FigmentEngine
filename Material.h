#ifndef FIGMENTENGINE_MATERIAL_H
#define FIGMENTENGINE_MATERIAL_H
#include <string>

#endif //FIGMENTENGINE_MATERIAL_H

struct Material {
    std::pmr::string name;
    std::string shaderVertex;
    std::string shaderFragment;
};