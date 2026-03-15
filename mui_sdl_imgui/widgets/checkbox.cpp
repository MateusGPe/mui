#include "checkbox.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Checkbox::Checkbox(const std::string &text) : text(text), checked(false) { App::assertMainThread(); }

    void Checkbox::renderControl()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);
        
        if (ImGui::Checkbox(text.c_str(), &checked)) {
            if (onToggledCb) onToggledCb();
        }
        
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    bool Checkbox::isChecked() const { return checked; }
    std::string Checkbox::getText() const { return text; }
    void Checkbox::setText(const std::string &t) { text = t; }

    CheckboxPtr Checkbox::setChecked(bool c) { checked = c; return self(); }
    CheckboxPtr Checkbox::onToggled(std::function<void()> cb) { onToggledCb = std::move(cb); return self(); }
} // namespace mui
