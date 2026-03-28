// widgets/coloredit.cpp
#include "coloredit.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

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
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        if (ImGui::ColorEdit4("##color", color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
        {
            onChangedSignal(getColor());
        }

        renderTooltip();

        ImGui::EndDisabled();
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

    ColorEditPtr ColorEdit::bind(std::shared_ptr<Observable<std::array<float, 4>>> observable)
    {
        auto c = observable->get();
        setColor(c[0], c[1], c[2], c[3]);
        m_connections.push_back(observable->onValueChanged.connect([this](const std::array<float, 4> &val) {
            mui::App::queueMain([this, val]() { this->setColor(val[0], val[1], val[2], val[3]); });
        }));
        m_connections.push_back(onChangedSignal.connect([observable](const std::array<float, 4> &val) {
            observable->set(val);
        }));
        return self();
    }

    ColorEditPtr ColorEdit::onChanged(std::function<void(const std::array<float, 4> &)> cb)
    {
        if (cb) m_connections.push_back(onChangedSignal.connect([cb](const std::array<float, 4> &val) { cb(val); }));
        return self();
    }
} // namespace mui