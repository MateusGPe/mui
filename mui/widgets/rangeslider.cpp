// widgets/rangeslider.cpp
#include "rangeslider.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"
#include <imgui_internal.h>

namespace mui
{
    RangeSlider::RangeSlider(float min, float max)
        : minBound(min), maxBound(max), currentMin(min), currentMax(max) { App::assertMainThread(); }

    void RangeSlider::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);

        auto render_slider = [&]
        {
            float values[2] = {currentMin, currentMax};
            if (ImGui::SliderFloat2("##range", values, minBound, maxBound, "%.2f"))
            {
                // Logic to prevent handles from crossing
                if (values[0] > values[1])
                    values[0] = values[1];
                currentMin = values[0];
                currentMax = values[1];
                if (onChangedCb)
                    onChangedCb(currentMin, currentMax);
            }
        };

        {
            ScopedItemWidth width;
            if (spanAvailWidth)
            {
                width.push(-FLT_MIN);
            }
            else if (useContainerWidth)
            {
                // Do nothing, will inherit width from container
            }
            render_slider();
        }

        renderTooltip();
    }

    RangeSliderPtr RangeSlider::setRange(float vMin, float vMax)
    {
        currentMin = vMin;
        currentMax = vMax;
        return self();
    }
    RangeSliderPtr RangeSlider::onChanged(std::function<void(float, float)> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }

    RangeSliderPtr RangeSlider::setUseContainerWidth(bool use)
    {
        useContainerWidth = use;
        return self();
    }
}
