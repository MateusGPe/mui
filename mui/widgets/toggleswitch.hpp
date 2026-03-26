// widgets/toggleswitch.hpp
#pragma once
#include "control.hpp"
#include "../core/observable.hpp"
#include "../core/signal.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace mui
{
    class ToggleSwitch;
    using ToggleSwitchPtr = std::shared_ptr<ToggleSwitch>;

    class ToggleSwitch : public Control<ToggleSwitch>
    {
    protected:
        std::string label;
        bool checked;
        float scale = 1.0f;

        explicit ToggleSwitch(const std::string &label);

    public:
        mui::Signal<bool> onToggledSignal;

        static ToggleSwitchPtr create(const std::string &label) { return std::shared_ptr<ToggleSwitch>(new ToggleSwitch(label)); }

        void renderControl() override;

        bool isChecked() const { return checked; }
        ToggleSwitchPtr setChecked(bool c);
        ToggleSwitchPtr setScale(float s);
        ToggleSwitchPtr bind(std::shared_ptr<Observable<bool>> observable);

        // Backward compatibility
        ToggleSwitchPtr onToggled(std::function<void(bool)> cb);
    };
} // namespace mui
