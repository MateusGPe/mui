#include "button.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Button::Button(const std::string &text) : text(text), type(ButtonType::Normal) { App::assertMainThread(); }

    void Button::render()
    {
        if (!visible)
            return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        if (useCustomColor)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorActive);
        }

        bool clicked = false;
        ImVec2 buttonSize(width > 0 ? width : (useContainerWidth ? ImGui::CalcItemWidth() : 0), height);

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

        if (clicked && onClickCb)
            onClickCb();

        renderTooltip();

        if (useCustomColor)
        {
            ImGui::PopStyleColor(3);
        }

        ImGui::EndDisabled();
        ImGui::PopID();
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
    ButtonPtr Button::setSize(float w, float h)
    {
        width = w;
        height = h;
        return self();
    }
    ButtonPtr Button::onClick(std::function<void()> cb)
    {
        onClickCb = std::move(cb);
        return self();
    }
    ButtonPtr Button::setUseContainerWidth(bool use)
    {
        useContainerWidth = use;
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
}