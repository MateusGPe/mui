// widgets/button.hpp
#pragma once
#include "control.hpp"
#include "../core/signal.hpp"
#include <string>
#include <functional>
#include <memory>
#include <imgui.h>

namespace mui
{
    class Button;
    using ButtonPtr = std::shared_ptr<Button>;

    enum class ButtonType
    {
        Normal,
        Small,
        Invisible,
        ArrowLeft,
        ArrowRight
    };

    class Button : public Control<Button>
    {
    protected:
        std::string getTypeName() const override { return "Button"; }
        std::string text;
        ButtonType type;
        bool useCustomColor = false;
        ImVec4 color;
        ImVec4 colorHovered;
        ImVec4 colorActive;

        explicit Button(const std::string &text);

    public:
        mui::Signal<> onClickSignal;

        static ButtonPtr create(const std::string &text) { return std::shared_ptr<Button>(new Button(text)); }

        void renderControl() override;

        std::string getText() const;
        ButtonPtr setText(const std::string &t);
        ButtonPtr setType(ButtonType t);
        ButtonPtr setColor(ImVec4 c, ImVec4 hover, ImVec4 active);
        
        // Backward compatibility for the fluent builder pattern
        ButtonPtr onClick(std::function<void()> cb);
    };
} // namespace mui
