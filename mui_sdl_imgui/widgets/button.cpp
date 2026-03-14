#include "button.hpp"
#include "app.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace mui
{
    Button::Button(const std::string &text) : text(text) { App::assertMainThread(); }

    void Button::render()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);
        
        // Unlike many other widgets, ImGui::Button doesn't automatically respect PushItemWidth.
        // We need to manually calculate the width and pass it as the size argument.
        // A width of 0 tells the button to auto-size, which is the default behavior
        // when no item width is pushed.
        ImVec2 buttonSize(0, 0);
        if (useContainerWidth)
        {
            buttonSize.x = ImGui::CalcItemWidth();
        }
        if (ImGui::Button(text.c_str(), buttonSize)) {
            if (onClickCb) onClickCb();
        }
        
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    std::string Button::getText() const { return text; }
    void Button::setText(const std::string &t) { text = t; }

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
} // namespace mui
