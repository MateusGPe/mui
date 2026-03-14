#pragma once
#include "control.hpp"
#include <functional>

namespace mui
{
    class Spinbox;
    using SpinboxPtr = std::shared_ptr<Spinbox>;

    class Spinbox : public Control
    {
        int minVal, maxVal, value;
        std::function<void()> onChangedCb;

    public:
        Spinbox(int min, int max);
        static SpinboxPtr create(int min, int max) { return std::make_shared<Spinbox>(min, max); }
        SpinboxPtr self() { return std::static_pointer_cast<Spinbox>(shared_from_this()); }

        void render() override;
        int getValue() const;
        SpinboxPtr setValue(int val);
        SpinboxPtr onChanged(std::function<void()> cb);
    };
} // namespace mui
