#include "label.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Label::Label(const std::string &text) : text(text) { App::assertMainThread(); }

    void Label::render()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);
        ImGui::TextUnformatted(text.c_str());
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    std::string Label::getText() const { return text; }
    LabelPtr Label::setText(const std::string &t) { text = t; return self(); }
} // namespace mui
