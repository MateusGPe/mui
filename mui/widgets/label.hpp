#pragma once
#include "control.hpp"
#include <string>
#include <memory>
#include <imgui.h>

namespace mui
{
    class Label;
    using LabelPtr = std::shared_ptr<Label>;

    enum class LabelFormat
    {
        Normal,
        Wrapped,
        Bullet,
        Disabled
    };

    class Label : public Control<Label>
    {
    protected:
        std::string text;
        LabelFormat format;

        bool useCustomColor = false;
        ImVec4 color;

    public:
        explicit Label(const std::string &text);
        static LabelPtr create(const std::string &text) { return std::make_shared<Label>(text); }

        void renderControl() override;

        std::string getText() const;
        LabelPtr setText(const std::string &t);
        LabelPtr setFormat(LabelFormat f);
        LabelPtr setWrapped(bool w);
        LabelPtr setColor(ImVec4 c);
    };
} // namespace mui
