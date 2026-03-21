#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>

namespace mui
{
    class Checkbox;
    using CheckboxPtr = std::shared_ptr<Checkbox>;

    class Checkbox : public Control<Checkbox>
    {
    protected:
        std::string text;
        bool checked;
        float scale = 1.0f;
        std::function<void(bool)> onToggledCb;

    public:
        explicit Checkbox(const std::string &text);
        static CheckboxPtr create(const std::string &text) { return std::make_shared<Checkbox>(text); }

        void renderControl() override;

        bool isChecked() const;
        std::string getText() const;
        CheckboxPtr setText(const std::string &t);

        CheckboxPtr setChecked(bool c);
        CheckboxPtr onToggled(std::function<void(bool)> cb);
        CheckboxPtr setScale(float s);
    };
} // namespace mui
