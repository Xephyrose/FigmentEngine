#ifndef FIGMENTENGINE_RESOURCE_H
#define FIGMENTENGINE_RESOURCE_H

struct Resource {
    virtual ~Resource() = default;
    virtual void ImGuiDraw() = 0;
};

#endif //FIGMENTENGINE_RESOURCE_H
