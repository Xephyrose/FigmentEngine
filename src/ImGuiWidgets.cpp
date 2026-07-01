#include "ImGuiWidgets.h"

#include <cstdio>

#include "EditorThemeManager.h"
#include "thirdparty/imgui/imgui.h"

namespace ImGui {
    void ColoredDragFloat(const char *label, float *v, const bool drawText)
    {
        PushID(label);

        PushStyleColor(ImGuiCol_Text, theme_text_color_light);
        PushStyleColor(ImGuiCol_TextDisabled, theme_text_color_disabled_light);
        PushStyleColor(ImGuiCol_FrameBg, theme_button_bg_light);
        PushStyleColor(ImGuiCol_FrameBgHovered, theme_button_bg_hovered_light);
        PushStyleColor(ImGuiCol_FrameBgActive, theme_button_bg_active_light);

        ImDrawList* drawList = GetWindowDrawList();
        const float lineHeight = GetFrameHeight();

        constexpr float itemWidth = 120.0f;
        const float boxWidth = drawText ? 16 : 6;
        const ImVec2 cursorPos = GetCursorScreenPos();

        const ImVec2 boxMin = cursorPos;
        const auto boxMax = ImVec2(cursorPos.x + boxWidth, cursorPos.y + lineHeight);

        drawList->AddRectFilled(
            boxMin,
            boxMax,
            GetColorU32(colors[0]),
            GetStyle().FrameRounding,
            ImDrawFlags_RoundCornersLeft
        );

        if (drawText) {
            const ImVec2 textSize = CalcTextSize("F");
            const auto textPos = ImVec2(
                cursorPos.x + (boxWidth - textSize.x) * 0.5f - (GetStyle().FrameRounding / 2),
                cursorPos.y + (lineHeight - textSize.y) * 0.5f
            );
            drawList->AddText(textPos, GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), "F");
        }

        SetCursorScreenPos(ImVec2(cursorPos.x + boxWidth - GetStyle().FrameRounding, cursorPos.y));
        SetNextItemWidth(itemWidth);

        char id[32];
        sprintf(id, "##%s_%d", label, 0);
        DragFloat(id, v);

        const auto lineStart = ImVec2(cursorPos.x + boxWidth, cursorPos.y + 2.0f);
        const auto lineEnd = ImVec2(cursorPos.x + boxWidth, cursorPos.y + lineHeight - 2.0f);
        drawList->AddLine(lineStart, lineEnd, GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.5f)), 1.0f);

        PopStyleColor(5);
        PopID();
    }
    void ColoredDragFloat2(const char *label, float v[2], const bool drawText)
    {
        PushID(label);

        PushStyleColor(ImGuiCol_Text, theme_text_color_light);
        PushStyleColor(ImGuiCol_TextDisabled, theme_text_color_disabled_light);
        PushStyleColor(ImGuiCol_FrameBg, theme_button_bg_light);
        PushStyleColor(ImGuiCol_FrameBgHovered, theme_button_bg_hovered_light);
        PushStyleColor(ImGuiCol_FrameBgActive, theme_button_bg_active_light);

        ImDrawList* drawList = GetWindowDrawList();
        const float lineHeight = GetFrameHeight();

        for (int i = 0; i < 2; i++)
        {
            constexpr float itemWidth = 120.0f;
            const float boxWidth = drawText ? 16 : 6;
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

            if (drawText) {
                const char* letters[2] = {"X", "Y"};
                const ImVec2 textSize = CalcTextSize(letters[i]);
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

            if (i < 1)
            {
                SameLine(0.0f, 1.0f);
            }
        }

        PopStyleColor(5);
        PopID();
    }
    void ColoredDragFloat3(const char *label, float v[3], const bool drawText)
{
    PushID(label);

    PushStyleColor(ImGuiCol_Text, theme_text_color_light);
    PushStyleColor(ImGuiCol_TextDisabled, theme_text_color_disabled_light);
    PushStyleColor(ImGuiCol_FrameBg, theme_button_bg_light);
    PushStyleColor(ImGuiCol_FrameBgHovered, theme_button_bg_hovered_light);
    PushStyleColor(ImGuiCol_FrameBgActive, theme_button_bg_active_light);

    ImDrawList* drawList = GetWindowDrawList();
    const float lineHeight = GetFrameHeight();

    for (int i = 0; i < 3; i++)
    {
        constexpr float itemWidth = 80.0f;
        constexpr float padding = 6.0f;
        const char* letters[3] = {"X", "YE", "ZEX"};

        // 1. Calculate text size first
        const ImVec2 textSize = CalcTextSize(letters[i]);

        // 2. Set box width based on text size
        float boxWidth = drawText ? padding + textSize.x + padding : padding;

        const ImVec2 cursorPos = GetCursorScreenPos();

        // 3. Draw the box (with the dynamically calculated width)
        ImVec2 boxMin = cursorPos;
        auto boxMax = ImVec2(cursorPos.x + boxWidth, cursorPos.y + lineHeight);

        drawList->AddRectFilled(
            boxMin,
            boxMax,
            GetColorU32(colors[i]),
            GetStyle().FrameRounding,
            ImDrawFlags_RoundCornersLeft
        );

        // 4. Draw the text centered in the box
        if (drawText) {
            auto textPos = ImVec2(
                cursorPos.x + (boxWidth - textSize.x) * 0.5f,
                cursorPos.y + (lineHeight - textSize.y) * 0.5f
            );
            drawList->AddText(textPos, GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), letters[i]);
        }

        // 5. Position the DragFloat after the box
        SetCursorScreenPos(ImVec2(cursorPos.x + boxWidth - GetStyle().FrameRounding, cursorPos.y));
        SetNextItemWidth(itemWidth);

        char id[32];
        sprintf(id, "##%s_%d", label, i);
        DragFloat(id, &v[i]);

        // 6. Draw the separator line
        auto lineStart = ImVec2(cursorPos.x + boxWidth, cursorPos.y + 2.0f);
        auto lineEnd = ImVec2(cursorPos.x + boxWidth, cursorPos.y + lineHeight - 2.0f);
        drawList->AddLine(lineStart, lineEnd, GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.5f)), 1.0f);

        if (i < 2)
        {
            SameLine(0.0f, 1.0f);
        }
    }

    PopStyleColor(5);
    PopID();
}
}
