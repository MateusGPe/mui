#include "progressbar.hpp"
#include "app.hpp"
#include <imgui.h>
#include <algorithm>

namespace mui
{
    ProgressBar::ProgressBar() : value(0.0f) { App::assertMainThread(); }

    void ProgressBar::renderControl()
    {
        if (!visible)
            return;
        ImGui::PushID(this);

        ImVec2 size(spanAvailWidth ? -FLT_MIN : 0, 0);
        const char *overlay = overlayText.empty() ? NULL : overlayText.c_str();

        ImGui::ProgressBar(value, size, overlay);

        renderTooltip();
        ImGui::PopID();
    }

    ProgressBarPtr ProgressBar::setValue(float v)
    {
        value = std::clamp(v, 0.0f, 1.0f);
        return self();
    }
    ProgressBarPtr ProgressBar::setOverlayText(const std::string &text)
    {
        overlayText = text;
        return self();
    }
    ProgressBarPtr ProgressBar::setSpanAvailWidth(bool span)
    {
        spanAvailWidth = span;
        return self();
    }
}
