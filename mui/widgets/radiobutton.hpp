// widgets/radiobutton.hpp
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
    class RadioButton;
    using RadioButtonPtr = std::shared_ptr<RadioButton>;

    class RadioButton : public Control<RadioButton>
    {
    protected:
        std::string getTypeName() const override { return "RadioButton"; }
        std::string text;

        std::shared_ptr<int> m_group_selection;
        int m_button_value = -1;
        bool m_checked = false;

        explicit RadioButton(const std::string &text);

    public:
        mui::Signal<bool> onToggledSignal;

        static RadioButtonPtr create(const std::string &text)
        {
            return std::shared_ptr<RadioButton>(new RadioButton(text));
        }

        void renderControl() override;

        bool isChecked() const;
        RadioButtonPtr setChecked(bool c);
        RadioButtonPtr bind(std::shared_ptr<Observable<bool>> observable);

        // Backward compatibility
        RadioButtonPtr onToggled(std::function<void(bool)> cb);

        static void group(std::initializer_list<RadioButtonPtr> buttons);
        void _setGroup(std::shared_ptr<int> selection, int value);
    };
} // namespace mui
