#pragma once
#include "control.hpp"
#include <string>

namespace mui
{
    class Label;
    using LabelPtr = std::shared_ptr<Label>;

    class Label : public Control
    {
        std::string text;
    public:
        Label(const std::string &text);
        static LabelPtr create(const std::string &text) { return std::make_shared<Label>(text); }
        
        LabelPtr self() { return std::static_pointer_cast<Label>(shared_from_this()); }

        void render() override;
        std::string getText() const;
        LabelPtr setText(const std::string &text);
    };
} // namespace mui
