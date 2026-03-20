#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>

namespace mui
{
    class SliderInt;
    using SliderIntPtr = std::shared_ptr<SliderInt>;

    class SliderInt : public Control
    {
    protected:
        int minVal;
        int maxVal;
        int value;
        std::string format;
        bool logarithmic = false;
        std::function<void()> onChangedCb;

        SliderIntPtr self() { return std::static_pointer_cast<SliderInt>(shared_from_this()); }

    public:
        SliderInt(int min, int max);
        static SliderIntPtr create(int min, int max) { return std::make_shared<SliderInt>(min, max); }

        void renderControl() override;

        int getValue() const;
        SliderIntPtr setValue(int v);
        SliderIntPtr setFormat(const std::string &f);
        SliderIntPtr setLogarithmic(bool l);
        SliderIntPtr onChanged(std::function<void()> cb);
    };

    class SliderFloat;
    using SliderFloatPtr = std::shared_ptr<SliderFloat>;

    class SliderFloat : public Control
    {
    protected:
        float minVal;
        float maxVal;
        float value;
        std::string format;
        bool logarithmic = false;
        std::function<void()> onChangedCb;

        SliderFloatPtr self() { return std::static_pointer_cast<SliderFloat>(shared_from_this()); }

    public:
        SliderFloat(float min, float max);
        static SliderFloatPtr create(float min, float max) { return std::make_shared<SliderFloat>(min, max); }

        void renderControl() override;

        float getValue() const;
        SliderFloatPtr setValue(float v);
        SliderFloatPtr setFormat(const std::string &f);
        SliderFloatPtr setLogarithmic(bool l);
        SliderFloatPtr onChanged(std::function<void()> cb);
    };
} // namespace mui
