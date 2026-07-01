#include "ImGuiWidgets.h"

namespace ImGui {
    void ColoredDragFloat3XYZ(const char *label, float (&v)[3]) {
        const char *xyz[3] = {"X", "Y", "Z"};
        ColoredDragFloat(label, v, xyz);
    }
    void ColoredDragFloat3RGB(const char *label, float (&v)[3]) {
        const char *xyz[3] = {"R", "G", "B"};
        ColoredDragFloat(label, v, xyz);
    }
}
