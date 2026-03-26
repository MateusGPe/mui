#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class Spinbox;
    using SpinboxPtr = std::shared_ptr<Spinbox>;

    class Spinbox : public Control<Spinbox>
    {
    protected:
        int minVal;
        int maxVal;
        int value;
        Spinbox(int min, int max);

    public:
        mui::Signal<int> onChangedSignal;

        static SpinboxPtr create(int min, int max) { return std::shared_ptr<Spinbox>(new Spinbox(min, max)); }

        void renderControl() override;

        int getValue() const;
        SpinboxPtr setValue(int v);
        SpinboxPtr bind(std::shared_ptr<Observable<int>> observable);
        SpinboxPtr onChanged(std::function<void()> cb);
    };
} // namespace mui
