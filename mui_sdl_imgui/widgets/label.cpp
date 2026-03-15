#include "label.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Label::Label(const std::string &text) : text(text), format(LabelFormat::Normal) { App::assertMainThread(); }

    void Label::render()
    {
        if (!visible)
            return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        if (useCustomColor)
            ImGui::PushStyleColor(ImGuiCol_Text, color);

        switch (format)
        {
        case LabelFormat::Wrapped:
            ImGui::TextWrapped("%s", text.c_str());
            break;
        case LabelFormat::Bullet:
            ImGui::BulletText("%s", text.c_str());
            break;
        case LabelFormat::Disabled:
            ImGui::TextDisabled("%s", text.c_str());
            break;
        case LabelFormat::Normal:
        default:
            ImGui::TextUnformatted(text.c_str());
            break;
        }

        renderTooltip();

        if (useCustomColor)
            ImGui::PopStyleColor();

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    std::string Label::getText() const { return text; }
    LabelPtr Label::setText(const std::string &t)
    {
        text = t;
        return self();
    }
    LabelPtr Label::setFormat(LabelFormat f)
    {
        format = f;
        return self();
    }
    LabelPtr Label::setColor(ImVec4 c)
    {
        color = c;
        useCustomColor = true;
        return self();
    }
}