#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>

namespace mui
{
    class Checkbox;
    using CheckboxPtr = std::shared_ptr<Checkbox>;

    class Checkbox : public Control
    {
    protected:
        std::string text;
        bool checked;
        std::function<void()> onToggledCb;

        CheckboxPtr self() { return std::static_pointer_cast<Checkbox>(shared_from_this()); }

    public:
        explicit Checkbox(const std::string &text);
        static CheckboxPtr create(const std::string &text) { return std::make_shared<Checkbox>(text); }

        void renderControl() override;

        bool isChecked() const;
        std::string getText() const;
        CheckboxPtr setText(const std::string &t);

        CheckboxPtr setChecked(bool c);
        CheckboxPtr onToggled(std::function<void()> cb);
    };
} // namespace mui
