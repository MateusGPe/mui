#include "slider.hpp"
#include <imgui.h>

namespace mui {
    Slider::Slider(int min, int max) : minVal(min), maxVal(max), value(min) {}
    void Slider::render() {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);
        if (ImGui::SliderInt("##slider", &value, minVal, maxVal)) {
            if (onChangedCb) onChangedCb();
        }
        ImGui::EndDisabled();
        ImGui::PopID();
    }
    int Slider::getValue() const { return value; }
    SliderPtr Slider::setValue(int v) { value = v; return self(); }
    SliderPtr Slider::onChanged(std::function<void()> cb) { onChangedCb = std::move(cb); return self(); }
}
