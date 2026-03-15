// widgets/separator.cpp
#include "separator.hpp"
#include <imgui.h>

namespace mui
{
    void Separator::renderControl()
    {
        if (!visible) return;
        ImGui::Separator();
        renderTooltip();
    }
} // namespace mui