// widgets/rangeslider.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>
#include <utility>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class RangeSlider;
    using RangeSliderPtr = std::shared_ptr<RangeSlider>;

    class RangeSlider : public Control<RangeSlider>
    {
    protected:
        std::string getTypeName() const override { return "RangeSlider"; }
        float minBound, maxBound;
        float currentMin, currentMax;
        std::string text;
        std::string format = "%.3f";
        RangeSlider(float min, float max);

    public:
        mui::Signal<float, float> onChangedSignal;

        static RangeSliderPtr create(float min, float max) { return std::shared_ptr<RangeSlider>(new RangeSlider(min, max)); }

        void renderControl() override;

        std::string getText() const { return text; }
        RangeSliderPtr setText(const std::string &t);
        std::pair<float, float> getRange() const { return {currentMin, currentMax}; }
        RangeSliderPtr setRange(float vMin, float vMax);
        RangeSliderPtr setFormat(const std::string &f);
        RangeSliderPtr bind(std::shared_ptr<Observable<std::pair<float, float>>> observable);
        RangeSliderPtr onChanged(std::function<void(float, float)> cb);
        RangeSliderPtr setUseContainerWidth(bool use);
    };
}
