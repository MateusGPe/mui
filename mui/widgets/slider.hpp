#pragma once
#include "control.hpp"
#include <string>
#include <memory>
#include "../core/observable.hpp"

namespace mui
{
    class SliderInt;
    using SliderIntPtr = std::shared_ptr<SliderInt>;

    class SliderInt : public Control<SliderInt>
    {
    protected:
        int minVal;
        int maxVal;
        int value;
        std::string format;
        bool logarithmic = false;
        SliderInt(int min, int max);

    public:
        static SliderIntPtr create(int min, int max) { return std::shared_ptr<SliderInt>(new SliderInt(min, max)); }

        void renderControl() override;

        int getValue() const;
        SliderIntPtr setValue(int v);
        SliderIntPtr setFormat(const std::string &f);
        SliderIntPtr setLogarithmic(bool l);

        mui::Signal<int> onChangedSignal;
        SliderIntPtr bind(std::shared_ptr<Observable<int>> observable);
        SliderIntPtr onChanged(std::function<void()> cb);
    };

    class SliderFloat;
    using SliderFloatPtr = std::shared_ptr<SliderFloat>;

    class SliderFloat : public Control<SliderFloat>
    {
    protected:
        float minVal;
        float maxVal;
        float value;
        std::string format;
        bool logarithmic = false;
        SliderFloat(float min, float max);

    public:
        static SliderFloatPtr create(float min, float max) { return std::shared_ptr<SliderFloat>(new SliderFloat(min, max)); }

        void renderControl() override;

        float getValue() const;
        SliderFloatPtr setValue(float v);
        SliderFloatPtr setFormat(const std::string &f);
        SliderFloatPtr setLogarithmic(bool l);

        mui::Signal<float> onChangedSignal;
        SliderFloatPtr bind(std::shared_ptr<Observable<float>> observable);
        SliderFloatPtr onChanged(std::function<void()> cb);
    };
} // namespace mui
