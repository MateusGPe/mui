#pragma once
#include "control.hpp"
#include <string>
#include <functional>

namespace mui
{
    class Checkbox;
    using CheckboxPtr = std::shared_ptr<Checkbox>;

    class Checkbox : public Control
    {
        std::string text;
        bool checked;
        std::function<void()> onToggledCb;

    public:
        Checkbox(const std::string &text);

        CheckboxPtr self() { return std::static_pointer_cast<Checkbox>(shared_from_this()); }

        void render() override;
        bool isChecked() const;
        std::string getText() const;
        void setText(const std::string &text);
        CheckboxPtr setChecked(bool checked);
        CheckboxPtr onToggled(std::function<void()> cb);
    };
} // namespace mui
