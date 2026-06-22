#ifndef FIGMENTENGINE_EDITORTHEMEMANAGER_H
#define FIGMENTENGINE_EDITORTHEMEMANAGER_H
#include "thirdparty/imgui/imgui.h"

static auto theme_text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
static auto theme_text_color_disabled = ImVec4(1.0f, 1.0f, 1.0f, 0.75f);

static auto theme_text_color_light = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
static auto theme_text_color_disabled_light = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);

static auto theme_button_bg = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
static auto theme_button_bg_hovered = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
static auto theme_button_bg_active = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);

static auto theme_button_bg_light = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
static auto theme_button_bg_hovered_light = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
static auto theme_button_bg_active_light = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);

struct EditorThemeManager {
    static void ApplyImGuiTheme();
};

#endif //FIGMENTENGINE_EDITORTHEMEMANAGER_H
