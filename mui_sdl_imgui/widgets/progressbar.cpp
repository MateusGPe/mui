#include "progressbar.hpp"
#include "app.hpp"
#include <imgui.h>
#include <algorithm>

namespace mui
{
    ProgressBar::ProgressBar() : value(0.0f), width(-1.0f), height(0.0f) { App::assertMainThread(); }

    void ProgressBar::render()
    {
        if (!visible)
            return;
        ImGui::PushID(this);

        ImVec2 size(width, height);
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
    ProgressBarPtr ProgressBar::setSize(float w, float h)
    {
        width = w;
        height = h;
        return self();
    }
}
