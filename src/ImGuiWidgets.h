#ifndef FIGMENTENGINE_IMGUIWIDGETS_H
#define FIGMENTENGINE_IMGUIWIDGETS_H
#include "thirdparty/imgui/imgui.h"

#include <cstdio>

#include "EditorThemeManager.h"

namespace ImGui {
    constexpr ImVec4 colors[4] = {
        ImVec4(0.6f, 0.1f, 0.0f, 1.0f),
        ImVec4(0.4f, 0.65f, 0.0f, 1.0f),
        ImVec4(0.1f, 0.5f, 1.0f, 1.0f),
        ImVec4(1.0f, 0.8f, 0.0f, 1.0f)
    };

    template<size_t N>
    void ColoredDragFloat(const char *label, float (&v)[N], const char* (&letters)[N])
    {
        PushID(label);

        PushStyleColor(ImGuiCol_Text, theme_text_color_light);
        PushStyleColor(ImGuiCol_TextDisabled, theme_text_color_disabled_light);
        PushStyleColor(ImGuiCol_FrameBg, theme_button_bg_light);
        PushStyleColor(ImGuiCol_FrameBgHovered, theme_button_bg_hovered_light);
        PushStyleColor(ImGuiCol_FrameBgActive, theme_button_bg_active_light);

        ImDrawList* drawList = GetWindowDrawList();
        const float lineHeight = GetFrameHeight();

        for (int i = 0; i < N; i++)
        {
            constexpr float itemWidth = 80.0f;
            constexpr float padding = 6.0f;

            const ImVec2 textSize = CalcTextSize(letters[i]);
            const float boxWidth = (letters != nullptr) ? padding + textSize.x + padding : padding;
            const ImVec2 cursorPos = GetCursorScreenPos();

            ImVec2 boxMin = cursorPos;
            auto boxMax = ImVec2(cursorPos.x + boxWidth, cursorPos.y + lineHeight);

            drawList->AddRectFilled(
                boxMin,
                boxMax,
                GetColorU32(colors[i]),
                GetStyle().FrameRounding,
                ImDrawFlags_RoundCornersLeft
            );

            if (letters != nullptr) {
                auto textPos = ImVec2(
                    cursorPos.x + (boxWidth - textSize.x) * 0.5f - (GetStyle().FrameRounding / 2),
                    cursorPos.y + (lineHeight - textSize.y) * 0.5f
                );
                drawList->AddText(textPos, GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), letters[i]);
            }

            SetCursorScreenPos(ImVec2(cursorPos.x + boxWidth - GetStyle().FrameRounding, cursorPos.y));
            SetNextItemWidth(itemWidth);

            char id[32];
            sprintf(id, "##%s_%d", label, i);
            DragFloat(id, &v[i]);

            auto lineStart = ImVec2(cursorPos.x + boxWidth, cursorPos.y + 2.0f);
            auto lineEnd = ImVec2(cursorPos.x + boxWidth, cursorPos.y + lineHeight - 2.0f);
            drawList->AddLine(lineStart, lineEnd, GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.5f)), 1.0f);

            if (i < N - 1)
            {
                SameLine(0.0f, 1.0f);
            }
        }

        PopStyleColor(5);
        PopID();
    }
    void ColoredDragFloat3XYZ(const char *label, float (&v)[3]);
    void ColoredDragFloat3RGB(const char *label, float (&v)[3]);
}

#endif //FIGMENTENGINE_IMGUIWIDGETS_H
