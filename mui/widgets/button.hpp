#pragma once
#include "control.hpp"
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
        std::string text;
        ButtonType type;
        bool useCustomColor = false;
        ImVec4 color;
        ImVec4 colorHovered;
        ImVec4 colorActive;

        std::function<void()> onClickCb;

    public:
        explicit Button(const std::string &text);
        static ButtonPtr create(const std::string &text) { return std::make_shared<Button>(text); }
        
        void renderControl() override;

        std::string getText() const;
        ButtonPtr setText(const std::string &t);
        ButtonPtr setType(ButtonType t);
        ButtonPtr onClick(std::function<void()> cb);
        ButtonPtr setColor(ImVec4 c, ImVec4 hover, ImVec4 active);
    };
} // namespace mui
