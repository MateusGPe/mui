#include "button.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Button::Button(const std::string &text) : text(text) { App::assertMainThread(); }

    void Button::render()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);
        
        if (ImGui::Button(text.c_str())) {
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
} // namespace mui
