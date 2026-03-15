// widgets/coloredit.cpp
#include "coloredit.hpp"
#include "../core/app.hpp"
#include <imgui.h>

namespace mui
{
    ColorEdit::ColorEdit(float r, float g, float b, float a)
    {
        color[0] = r;
        color[1] = g;
        color[2] = b;
        color[3] = a;
        App::assertMainThread();
    }

    void ColorEdit::renderControl()
    {
        if (!visible)
            return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        if (ImGui::ColorEdit4("##color", color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
        {
            if (onChangedCb)
                onChangedCb();
        }

        renderTooltip();

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    std::array<float, 4> ColorEdit::getColor() const
    {
        return {color[0], color[1], color[2], color[3]};
    }

    ColorEditPtr ColorEdit::setColor(float r, float g, float b, float a)
    {
        color[0] = r;
        color[1] = g;
        color[2] = b;
        color[3] = a;
        return self();
    }

    ColorEditPtr ColorEdit::onChanged(std::function<void()> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }
} // namespace mui