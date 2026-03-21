#include "slider.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    // Slider Int
    SliderInt::SliderInt(int min, int max) : minVal(min), maxVal(max), value(min), format("%d") { App::assertMainThread(); }
    void SliderInt::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiSliderFlags flags = logarithmic ? ImGuiSliderFlags_Logarithmic : 0;
        if (ImGui::SliderInt("##slider", &value, minVal, maxVal, format.c_str(), flags))
        {
            if (onChangedCb)
                onChangedCb();
        }

        renderTooltip();
        ImGui::EndDisabled();
    }
    int SliderInt::getValue() const { return value; }
    SliderIntPtr SliderInt::setValue(int v)
    {
        value = v;
        return self();
    }
    SliderIntPtr SliderInt::setFormat(const std::string &f)
    {
        format = f;
        return self();
    }
    SliderIntPtr SliderInt::setLogarithmic(bool l)
    {
        logarithmic = l;
        return self();
    }
    SliderIntPtr SliderInt::onChanged(std::function<void()> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }

    // Slider Float
    SliderFloat::SliderFloat(float min, float max) : minVal(min), maxVal(max), value(min), format("%.3f") { App::assertMainThread(); }
    void SliderFloat::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiSliderFlags flags = logarithmic ? ImGuiSliderFlags_Logarithmic : 0;
        if (ImGui::SliderFloat("##slider", &value, minVal, maxVal, format.c_str(), flags))
        {
            if (onChangedCb)
                onChangedCb();
        }

        renderTooltip();
        ImGui::EndDisabled();
    }
    float SliderFloat::getValue() const { return value; }
    SliderFloatPtr SliderFloat::setValue(float v)
    {
        value = v;
        return self();
    }
    SliderFloatPtr SliderFloat::setFormat(const std::string &f)
    {
        format = f;
        return self();
    }
    SliderFloatPtr SliderFloat::setLogarithmic(bool l)
    {
        logarithmic = l;
        return self();
    }
    SliderFloatPtr SliderFloat::onChanged(std::function<void()> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }
}