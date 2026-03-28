// widgets/iconstack.cpp
#include "iconstack.hpp"
#include "app.hpp"
#include <imgui.h>
#include <imgui_internal.h> // For ButtonBehavior, RenderFrame, etc.
#include "../core/scoped.hpp"
#include <algorithm> // For std::max

namespace mui
{
    IconStack::IconStack() { App::assertMainThread(); }

    void IconStack::renderControl()
    {
        if (!visible || icons.empty())
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::EndDisabled();
            return;
        }

        const ImGuiStyle &style = ImGui::GetStyle();
        const float frame_height = ImGui::GetFrameHeight();

        // --- Pass 1: Find max icon width for alignment ---
        // This ensures all icons are centered relative to the same width, preventing visual shifts.
        float max_icon_width = 0.0f;
        for (const auto &icon : icons)
        {
            max_icon_width = std::max(max_icon_width, ImGui::CalcTextSize(icon.icon.c_str()).x);
        }

        // --- Pass 2: Render buttons with manual text alignment ---
        // Make buttons wide enough to fit the largest icon plus padding.
        const float button_width = max_icon_width + style.FramePadding.x * 2.0f;
        const float button_height = frame_height;

        ImGui::BeginGroup(); // Group the buttons to treat them as a single item for layout
        for (size_t i = 0; i < icons.size(); ++i)
        {
            ScopedID icon_id((int)i);
            const ImVec2 pos = window->DC.CursorPos;
            const ImRect bb(pos, ImVec2(pos.x + button_width, pos.y + button_height));
            const ImGuiID button_id_ = window->GetID(icons[i].icon.c_str());

            ImGui::ItemSize(bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(bb, button_id_))
            {
                if (i < icons.size() - 1)
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                continue;
            }

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, button_id_, &hovered, &held);
            if (pressed)
            {
                onIconClickedSignal((int)i);
            }

            // Render background like a standard button
            const ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered
                                                                                        : ImGuiCol_Button);
            ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

            // Render icon text centered within the calculated `button_width`
            const ImVec2 icon_size = ImGui::CalcTextSize(icons[i].icon.c_str());
            const ImVec2 icon_pos(bb.Min.x + (button_width - icon_size.x) * 0.5f, bb.Min.y + (button_height - icon_size.y) * 0.5f);

            ScopedColor text_color;
            if (icons[i].text_hover)
                text_color.push(
                    ImGuiCol_Text,
                    ImGui::GetColorU32(
                        held      ? ImGuiCol_TextDisabled
                        : hovered ? ImGuiCol_Text
                                  : ImGuiCol_TextDisabled));
            ImGui::RenderText(icon_pos, icons[i].icon.c_str());

            if (!icons[i].tooltip.empty() && ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", icons[i].tooltip.c_str());
            }
            if (i < icons.size() - 1)
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
        }
        ImGui::EndGroup();
        ImGui::EndDisabled();
    }

    IconStackPtr IconStack::add(const std::string &icon, std::function<void()> onClick, const std::string &tip, bool textHover)
    {
        int index = (int)icons.size();
        icons.push_back({icon, tip, textHover});

        if (onClick)
        {
            m_connections.push_back(onIconClickedSignal.connect([index, cb = std::move(onClick)](int clickedIndex)
                                                                {
                if (clickedIndex == index) cb(); }));
        }

        return self();
    }
}
