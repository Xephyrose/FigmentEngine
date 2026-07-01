#ifndef FIGMENTENGINE_IMGUIWIDGETS_H
#define FIGMENTENGINE_IMGUIWIDGETS_H
#include "thirdparty/imgui/imgui.h"

namespace ImGui {
    void ColoredDragFloat(const char* label, float *v, const char* letters);
    void ColoredDragFloat2(const char* label, float v[2], const char* letters[2] = nullptr);
    void ColoredDragFloat3(const char* label, float v[3], const char* letters[3] = nullptr);
    void ColoredDragFloat3XYZ(const char* label, float v[3]);
    void ColoredDragFloat3RGB(const char* label, float v[3]);

    constexpr ImVec4 colors[4] = {
        ImVec4(0.6f, 0.1f, 0.0f, 1.0f),
        ImVec4(0.4f, 0.65f, 0.0f, 1.0f),
        ImVec4(0.1f, 0.5f, 1.0f, 1.0f),
        ImVec4(1.0f, 0.8f, 0.0f, 1.0f)
    };
}

#endif //FIGMENTENGINE_IMGUIWIDGETS_H
