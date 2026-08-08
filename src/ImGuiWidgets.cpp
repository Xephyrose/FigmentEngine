#include "ImGuiWidgets.h"

namespace ImGui {
    void ColoredDragFloat1(const char *label, float &v, const char* letters) {
        float num[1] = {v};
        const char* let[1] = {letters};
        ColoredDragFloat(label, num, let);
        v = num[0];
    }
    void ColoredDragFloat3XYZ(const char *label, float (&v)[3]) {
        const char *xyz[3] = {"X", "Y", "Z"};
        ColoredDragFloat(label, v, xyz);
    }
    void ColoredDragFloat3RGB(const char *label, float (&v)[3]) {
        const char *xyz[3] = {"R", "G", "B"};
        ColoredDragFloat(label, v, xyz);
    }
    void ColoredDragFloat4RGBA(const char *label, float (&v)[4]) {
        const char *xyzw[4] = {"R", "G", "B", "A"};
        ColoredDragFloat(label, v, xyzw);
    }
}
