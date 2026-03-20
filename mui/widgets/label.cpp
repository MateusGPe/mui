#include "label.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Label::Label(const std::string &text) : text(text), format(LabelFormat::Normal) { App::assertMainThread(); }

    void Label::renderControl()
    {
        if (!visible)
            return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        if (useCustomColor)
            ImGui::PushStyleColor(ImGuiCol_Text, color);

        if (spanAvailWidth)
        {
            // To make a simple text label span, we can use TextWrapped and push the item width.
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::TextWrapped("%s", text.c_str());
            ImGui::PopItemWidth();
        }
        else
        {
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
    LabelPtr Label::setWrapped(bool w)
    {
        format = w ? LabelFormat::Wrapped : LabelFormat::Normal;
        return self();
    }
    LabelPtr Label::setColor(ImVec4 c)
    {
        color = c;
        useCustomColor = true;
        return self();
    }
}