#pragma once
#include "control.hpp"
#include <string>
#include <functional>

namespace mui
{
    class Button;
    using ButtonPtr = std::shared_ptr<Button>;

    class Button : public Control
    {
        std::string text;
        std::function<void()> onClickCb;
        bool useContainerWidth = false;

    public:
        Button(const std::string &text);
        static ButtonPtr create(const std::string &text) { return std::make_shared<Button>(text); }
        
        ButtonPtr self() { return std::static_pointer_cast<Button>(shared_from_this()); }
        
        void render() override;
        std::string getText() const;
        void setText(const std::string &text);
        ButtonPtr onClick(std::function<void()> cb);
        ButtonPtr setUseContainerWidth(bool use);
    };
} // namespace mui
