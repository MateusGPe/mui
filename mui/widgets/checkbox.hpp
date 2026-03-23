// widgets/checkbox.hpp
#pragma once
#include "control.hpp"
#include "../core/observable.hpp"
#include "../core/signal.hpp"
#include <string>
#include <memory>
#include <vector>

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

        explicit Checkbox(const std::string &text);

    public:
        mui::Signal<bool> onToggledSignal;

        static CheckboxPtr create(const std::string &text) { return std::shared_ptr<Checkbox>(new Checkbox(text)); }

        void renderControl() override;

        bool isChecked() const;
        std::string getText() const;
        CheckboxPtr setText(const std::string &t);

        CheckboxPtr setChecked(bool c);
        CheckboxPtr setScale(float s);
        CheckboxPtr bind(std::shared_ptr<Observable<bool>> observable);
        CheckboxPtr onToggled(std::function<void(bool)> cb);
    };
} // namespace mui
