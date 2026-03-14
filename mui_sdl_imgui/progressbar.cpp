#include "progressbar.hpp"
#include <imgui.h>
#include <algorithm>

namespace mui {
    ProgressBar::ProgressBar() : value(0) {}
    void ProgressBar::render() {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::ProgressBar(value / 100.0f);
        ImGui::PopID();
    }
    ProgressBarPtr ProgressBar::setValue(int v) { value = std::clamp(v, 0, 100); return self(); }
}
