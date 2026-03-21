// widgets/iconbutton.cpp
#include "iconbutton.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"
#include <imgui_internal.h>
#include <algorithm>
#include <cfloat>

namespace mui
{
    IconButton::IconButton(const std::string &text) : text(text) { App::assertMainThread(); }

    void IconButton::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::EndDisabled();
            return;
        }

        ImGuiStyle &style = ImGui::GetStyle();

        // 1. Calculate the ideal dimensions
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float w = width;
        if (spanAvailWidth)
            w = avail.x;
        else if (useContainerWidth)
            w = ImGui::CalcItemWidth();
        ImVec2 actualSize(w, height);

        float contentWidth = 0.0f;
        float calculatedIconWidth = iconSize.x;

        if (!iconText.empty())
        {
            calculatedIconWidth = ImGui::GetFont()->CalcTextSizeA(iconSize.y, FLT_MAX, 0.0f, iconText.c_str()).x;
        }

        if (iconTex || !iconText.empty())
            contentWidth += calculatedIconWidth;
        if (!text.empty())
        {
            if (contentWidth > 0.0f)
                contentWidth += style.ItemInnerSpacing.x;
            contentWidth += ImGui::CalcTextSize(text.c_str()).x;
        }

        if (actualSize.x <= 0)
            actualSize.x = contentWidth + style.FramePadding.x * 2.0f;
        if (actualSize.y <= 0)
            actualSize.y = std::max((iconTex || !iconText.empty() ? iconSize.y : 0.0f), ImGui::GetFontSize()) + style.FramePadding.y * 2.0f;

        // 2. Register bounding box
        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos, ImVec2(pos.x + actualSize.x, pos.y + actualSize.y));

        // Generate a unique ID once and reuse it for ItemAdd and ButtonBehavior.
        // Use the button's text if available, otherwise rely on the ID stack for uniqueness.
        const ImGuiID button_id = window->GetID(text.empty() ? "##icon_button" : text.c_str());

        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, button_id))
        {
            ImGui::EndDisabled();
            return;
        }

        // 3. Process Events
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, button_id, &hovered, &held);

        if (pressed)
            onClickSignal();
        if (hovered && ImGui::IsMouseDoubleClicked(0))
            onDoubleClickSignal();

        // 4. Draw Background
        ImU32 col_bg = 0;
        if (held)
            col_bg = ImGui::GetColorU32(ImGuiCol_HeaderActive);
        else if (hovered)
            col_bg = ImGui::GetColorU32(ImGuiCol_HeaderHovered);
        else if (selected)
            col_bg = ImGui::GetColorU32(ImGuiCol_Header);

        if (col_bg != 0)
        {
            window->DrawList->AddRectFilled(bb.Min, bb.Max, col_bg, style.FrameRounding);
        }

        // 5. Render Content Layout
        if (layout == IconButtonLayout::Vertical)
        {
            float iconX = pos.x + (actualSize.x - calculatedIconWidth) / 2.0f;
            float iconY = pos.y + style.FramePadding.y;

            if (iconTex)
            {
                window->DrawList->AddImage(iconTex, ImVec2(iconX, iconY), ImVec2(iconX + iconSize.x, iconY + iconSize.y));
            }
            else if (!iconText.empty())
            {
                window->DrawList->AddText(ImGui::GetFont(), iconSize.y, ImVec2(iconX, iconY), ImGui::GetColorU32(ImGuiCol_Text), iconText.c_str());
            }

            if (!text.empty())
            {
                ImVec2 tSize = ImGui::CalcTextSize(text.c_str(), nullptr, true, actualSize.x);
                float textX = pos.x + (actualSize.x - tSize.x) / 2.0f;
                float textY = pos.y + iconSize.y + style.FramePadding.y + (actualSize.y - iconSize.y - style.FramePadding.y * 2.0f - tSize.y) / 2.0f;
                window->DrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_Text), text.c_str(), nullptr, actualSize.x);
            }
        }
        else
        {
            float currentX = pos.x + (actualSize.x - contentWidth) / 2.0f; // Centered Horizontal
            float centerY = pos.y + actualSize.y / 2.0f;

            if (iconTex)
            {
                float iconY = centerY - iconSize.y / 2.0f;
                window->DrawList->AddImage(iconTex, ImVec2(currentX, iconY), ImVec2(currentX + iconSize.x, iconY + iconSize.y));
                currentX += iconSize.x + style.ItemInnerSpacing.x;
            }
            else if (!iconText.empty())
            {
                float iconY = centerY - iconSize.y / 2.0f;
                window->DrawList->AddText(ImGui::GetFont(), iconSize.y, ImVec2(currentX, iconY), ImGui::GetColorU32(ImGuiCol_Text), iconText.c_str());
                currentX += calculatedIconWidth + style.ItemInnerSpacing.x;
            }

            if (!text.empty())
            {
                ImVec2 tSize = ImGui::CalcTextSize(text.c_str());
                float textY = centerY - tSize.y / 2.0f;
                window->DrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(currentX, textY), ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
            }
        }

        renderTooltip();
        ImGui::EndDisabled();
    }

    IconButtonPtr IconButton::setText(const std::string &t)
    {
        text = t;
        return self();
    }
    IconButtonPtr IconButton::setIconText(const std::string &t)
    {
        iconText = t;
        return self();
    }
    IconButtonPtr IconButton::setIconTexture(ImTextureID tex)
    {
        iconTex = tex;
        return self();
    }
    IconButtonPtr IconButton::setLayout(IconButtonLayout l)
    {
        layout = l;
        return self();
    }
    IconButtonPtr IconButton::setSelected(bool s)
    {
        selected = s;
        return self();
    }
    IconButtonPtr IconButton::setIconSize(float w, float h)
    {
        iconSize = ImVec2(w, h);
        return self();
    }
    IconButtonPtr IconButton::onClick(std::function<void()> cb)
    {
        if (cb)
            m_connections.push_back(onClickSignal.connect(std::move(cb)));
        return self();
    }
    IconButtonPtr IconButton::onDoubleClick(std::function<void()> cb)
    {
        if (cb)
            m_connections.push_back(onDoubleClickSignal.connect(std::move(cb)));
        return self();
    }

    IconButtonPtr IconButton::setUseContainerWidth(bool use)
    {
        useContainerWidth = use;
        return self();
    }
} // namespace mui
