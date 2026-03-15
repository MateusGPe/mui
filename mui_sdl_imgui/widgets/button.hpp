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

    class Button : public Control
    {
    protected:
        std::string text;
        ButtonType type;
        float width = 0.0f;
        float height = 0.0f;
        bool useContainerWidth = false;
        
        bool useCustomColor = false;
        ImVec4 color;
        ImVec4 colorHovered;
        ImVec4 colorActive;

        std::function<void()> onClickCb;

        ButtonPtr self() { return std::static_pointer_cast<Button>(shared_from_this()); }

    public:
        explicit Button(const std::string &text);
        static ButtonPtr create(const std::string &text) { return std::make_shared<Button>(text); }
        
        void render() override;

        std::string getText() const;
        ButtonPtr setText(const std::string &t);
        ButtonPtr setType(ButtonType t);
        ButtonPtr setSize(float w, float h);
        
        ButtonPtr onClick(std::function<void()> cb);
        ButtonPtr setUseContainerWidth(bool use);
        ButtonPtr setColor(ImVec4 c, ImVec4 hover, ImVec4 active);
    };
} // namespace mui
