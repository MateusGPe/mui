// widgets/toggleswitch.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>

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
        std::function<void(bool)> onToggledCb;

    public:
        explicit ToggleSwitch(const std::string &label);
        static ToggleSwitchPtr create(const std::string &label) { return std::make_shared<ToggleSwitch>(label); }

        void renderControl() override;

        bool isChecked() const { return checked; }
        ToggleSwitchPtr setChecked(bool c);
        ToggleSwitchPtr onToggled(std::function<void(bool)> cb);
        ToggleSwitchPtr setScale(float s);
    };
} // namespace mui
