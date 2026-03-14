#pragma once
#include "control.hpp"
#include <functional>

namespace mui
{
    class Slider;
    using SliderPtr = std::shared_ptr<Slider>;

    class Slider : public Control
    {
        int minVal, maxVal, value;
        std::function<void()> onChangedCb;

    public:
        Slider(int min, int max);
        static SliderPtr create(int min, int max) { return std::make_shared<Slider>(min, max); }
        SliderPtr self() { return std::static_pointer_cast<Slider>(shared_from_this()); }

        void render() override;
        int getValue() const;
        SliderPtr setValue(int val);
        SliderPtr onChanged(std::function<void()> cb);
    };
} // namespace mui
