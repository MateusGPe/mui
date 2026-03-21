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
            onChangedSignal(value);
        }
        ImGui::EndDisabled();
    }
    int Spinbox::getValue() const { return value; }
    SpinboxPtr Spinbox::setValue(int v)
    {
        value = v;
        return self();
    }

    SpinboxPtr Spinbox::bind(std::shared_ptr<Observable<int>> observable)
    {
        setValue(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const int &val) {
            mui::App::queueMain([this, val]() { this->setValue(val); });
        }));
        m_connections.push_back(onChangedSignal.connect([observable](int val) {
            observable->set(val);
        }));
        return self();
    }

    SpinboxPtr Spinbox::onChanged(std::function<void()> cb)
    {
        if (cb) m_connections.push_back(onChangedSignal.connect([cb](int) { cb(); }));
        return self();
    }
}
