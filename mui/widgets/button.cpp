// widgets/button.cpp
#include "button.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"
#include <algorithm>

namespace mui
{
    Button::Button(const std::string &text) : text(text), type(ButtonType::Normal) { App::assertMainThread(); }

    void Button::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        if (useCustomColor)
        {
            ScopedColor colors;
            colors.push(ImGuiCol_Button, color);
            colors.push(ImGuiCol_ButtonHovered, colorHovered);
            colors.push(ImGuiCol_ButtonActive, colorActive);
        }

        bool clicked = false;
        ImVec2 buttonSize(width, height);
        if (spanAvailWidth)
        {
            buttonSize.x = ImGui::GetContentRegionAvail().x;
        }
        else if (useContainerWidth)
        {
            buttonSize.x = ImGui::CalcItemWidth();
        }

        if (buttonSize.x <= 0.0f)
            buttonSize.x = ImGui::CalcTextSize(text.c_str(), NULL, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        if (buttonSize.y <= 0.0f)
            buttonSize.y = ImGui::GetFrameHeight();

        buttonSize.x = std::clamp(buttonSize.x, minSize.x, maxSize.x);
        buttonSize.y = std::clamp(buttonSize.y, minSize.y, maxSize.y);

        switch (type)
        {
        case ButtonType::Small:
            clicked = ImGui::SmallButton(text.c_str());
            break;
        case ButtonType::Invisible:
            clicked = ImGui::InvisibleButton(text.c_str(), buttonSize);
            break;
        case ButtonType::ArrowLeft:
            clicked = ImGui::ArrowButton(text.c_str(), ImGuiDir_Left);
            break;
        case ButtonType::ArrowRight:
            clicked = ImGui::ArrowButton(text.c_str(), ImGuiDir_Right);
            break;
        case ButtonType::Normal:
        default:
            clicked = ImGui::Button(text.c_str(), buttonSize);
            break;
        }

        if (clicked)
            onClickSignal();

        renderTooltip();

        ImGui::EndDisabled();
    }

    std::string Button::getText() const { return text; }
    ButtonPtr Button::setText(const std::string &t)
    {
        text = t;
        return self();
    }
    ButtonPtr Button::setType(ButtonType t)
    {
        type = t;
        return self();
    }

    ButtonPtr Button::setColor(ImVec4 c, ImVec4 hover, ImVec4 active)
    {
        color = c;
        colorHovered = hover;
        colorActive = active;
        useCustomColor = true;
        return self();
    }

    ButtonPtr Button::onClick(std::function<void()> cb)
    {
        if (cb)
            m_connections.push_back(onClickSignal.connect(std::move(cb)));
        return self();
    }
}
