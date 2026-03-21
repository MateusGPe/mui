#include "spinbox.hpp"
#include "app.hpp"
#include <imgui.h>
#include <algorithm>
#include "../core/scoped.hpp"

namespace mui
{
    Spinbox::Spinbox(int min, int max) : minVal(min), maxVal(max), value(min) { App::assertMainThread(); }
    void Spinbox::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);
        if (ImGui::InputInt("##spin", &value, 1, 100))
        {
            value = std::clamp(value, minVal, maxVal);
            if (onChangedCb)
                onChangedCb();
        }
        ImGui::EndDisabled();
    }
    int Spinbox::getValue() const { return value; }
    SpinboxPtr Spinbox::setValue(int v)
    {
        value = v;
        return self();
    }
    SpinboxPtr Spinbox::onChanged(std::function<void()> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }
}
