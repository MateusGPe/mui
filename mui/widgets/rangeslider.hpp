// widgets/rangeslider.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>

namespace mui
{
    class RangeSlider;
    using RangeSliderPtr = std::shared_ptr<RangeSlider>;

    class RangeSlider : public Control<RangeSlider>
    {
    protected:
        float minBound, maxBound;
        float currentMin, currentMax;
        std::function<void(float, float)> onChangedCb;

    public:
        RangeSlider(float min, float max);
        static RangeSliderPtr create(float min, float max) { return std::make_shared<RangeSlider>(min, max); }

        void renderControl() override;

        std::pair<float, float> getRange() const { return {currentMin, currentMax}; }
        RangeSliderPtr setRange(float vMin, float vMax);
        RangeSliderPtr onChanged(std::function<void(float, float)> cb);
        RangeSliderPtr setUseContainerWidth(bool use);
    };
}
