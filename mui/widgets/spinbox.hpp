#pragma once
#include "control.hpp"
#include <functional>
#include <memory>

namespace mui
{
    class Spinbox;
    using SpinboxPtr = std::shared_ptr<Spinbox>;

    class Spinbox : public Control
    {
    protected:
        int minVal;
        int maxVal;
        int value;
        std::function<void()> onChangedCb;

        SpinboxPtr self() { return std::static_pointer_cast<Spinbox>(shared_from_this()); }

    public:
        Spinbox(int min, int max);
        static SpinboxPtr create(int min, int max) { return std::make_shared<Spinbox>(min, max); }

        void renderControl() override;

        int getValue() const;
        SpinboxPtr setValue(int v);
        SpinboxPtr onChanged(std::function<void()> cb);
    };
} // namespace mui
