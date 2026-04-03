#include "label.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Label::Label(const std::string &text)
        : text(text), format(LabelFormat::Normal)
    {
        App::assertMainThread();
        inlineShadowEnabled = false;
    }

    void Label::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ScopedColor scoped_color;
        if (useCustomColor)
            scoped_color.push(ImGuiCol_Text, color);

        if (spanAvailWidth)
        {
            ScopedItemWidth width(-FLT_MIN);
            ImGui::TextWrapped("%s", text.c_str());
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

        ImGui::EndDisabled();
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
} // namespace mui