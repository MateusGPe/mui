#include "spinbox.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <algorithm>
#include <imgui.h>

namespace mui
{
    Spinbox::Spinbox(int min, int max) : minVal(min), maxVal(max), value(min)
    {
        App::assertMainThread();
    }
    void Spinbox::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImVec2 final_size = ApplySizeConstraints(ImVec2(0.0f, 0.0f));
        ScopedItemWidth item_width;
        if (final_size.x > 0.0f)
            item_width.push(final_size.x);

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
        addConnection(
            observable->onValueChanged.connect([this](const int &val)
                                               { mui::App::queueMain([this, val]()
                                                                     { this->setValue(val); }); }));
        addConnection(
            onChangedSignal.connect([observable](int val)
                                    { observable->set(val); }));
        return self();
    }

    SpinboxPtr Spinbox::onChanged(std::function<void()> cb)
    {
        if (cb)
            addConnection(onChangedSignal.connect([cb](int)
                                                            { cb(); }));
        return self();
    }
} // namespace mui
