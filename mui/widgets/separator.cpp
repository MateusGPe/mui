// widgets/separator.cpp
#include "separator.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include "../core/scoped.hpp"

namespace mui
{
    void Separator::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);

        switch (type)
        {
        case SeparatorType::Native:
            ImGui::Separator();
            break;
        case SeparatorType::Text:
            ImGui::SeparatorText(text.c_str());
            break;
        case SeparatorType::Custom:
        default:
            renderCustomSeparator();
            break;
        }

        renderTooltip();
    }

    void Separator::renderCustomSeparator()
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        const ImGuiStyle &style = ImGui::GetStyle();
        ImU32 col = useCustomColor ? ImGui::GetColorU32(color) : ImGui::GetColorU32(ImGuiCol_Separator);

        if (orientation == SeparatorOrientation::Horizontal)
        {
            float w = ImGui::GetContentRegionAvail().x;
            const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.y + thickness));
            ImGui::ItemSize(bb.GetSize());
            if (ImGui::ItemAdd(bb, 0))
            {
                if (isRect)
                {
                    window->DrawList->AddRectFilled(bb.Min, bb.Max, col);
                }
                else
                {
                    window->DrawList->AddLine(ImVec2(bb.Min.x, bb.GetCenter().y), ImVec2(bb.Max.x, bb.GetCenter().y), col, thickness);
                }
            }
        }
        else // Vertical
        {
            ImGui::SameLine(0, style.ItemSpacing.x);
            float h = ImGui::GetContentRegionAvail().y;
            const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + thickness, window->DC.CursorPos.y + h));
            ImGui::ItemSize(bb.GetSize());
            if (ImGui::ItemAdd(bb, 0))
            {
                if (isRect)
                {
                    window->DrawList->AddRectFilled(bb.Min, bb.Max, col);
                }
                else
                {
                    window->DrawList->AddLine(ImVec2(bb.GetCenter().x, bb.Min.y), ImVec2(bb.GetCenter().x, bb.Max.y), col, thickness);
                }
            }
            ImGui::SameLine(0, style.ItemSpacing.x);
        }
    }

    SeparatorPtr Separator::setType(SeparatorType t)
    {
        type = t;
        return self();
    }
    SeparatorPtr Separator::setOrientation(SeparatorOrientation o)
    {
        orientation = o;
        return self();
    }
    SeparatorPtr Separator::setThickness(float t)
    {
        thickness = t;
        return self();
    }
    SeparatorPtr Separator::setAsRect(bool rect)
    {
        isRect = rect;
        return self();
    }

    SeparatorPtr Separator::setColor(ImVec4 c)
    {
        color = c;
        useCustomColor = true;
        return self();
    }

    SeparatorPtr Separator::setText(const std::string &s)
    {
        text = s;
        // Automatically switch to Text type if text is set, for convenience.
        // Note: SeparatorType::Text does not respect item width and may overflow
        // in containers like Card that use PushItemWidth for padding.
        if (!text.empty())
        {
            type = SeparatorType::Text;
        }
        return self();
    }

    std::string Separator::getText() const { return text; }
} // namespace mui