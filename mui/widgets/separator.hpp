// widgets/separator.hpp
#pragma once
#include "control.hpp"
#include <memory>
#include <imgui.h>

namespace mui
{
    class Separator;
    using SeparatorPtr = std::shared_ptr<Separator>;

    enum class SeparatorOrientation
    {
        Horizontal,
        Vertical
    };

    enum class SeparatorType
    {
        Custom, // Manually drawn (default)
        Native, // Uses ImGui::SeparatorEx
        Text    // Uses ImGui::SeparatorText
    };

    class Separator : public Control<Separator>
    {
    protected:
        void renderCustomSeparator();
        SeparatorType type = SeparatorType::Custom;
        SeparatorOrientation orientation = SeparatorOrientation::Horizontal;
        float thickness = 1.0f;
        std::string text;
        bool isRect = false;
        bool useCustomColor = false;
        ImVec4 color;
        Separator() = default;

    public:
        static SeparatorPtr create() { return std::shared_ptr<Separator>(new Separator()); }
        void renderControl() override;

        SeparatorPtr setType(SeparatorType t);
        SeparatorPtr setOrientation(SeparatorOrientation o);
        SeparatorPtr setThickness(float t);
        SeparatorPtr setAsRect(bool rect);
        SeparatorPtr setColor(ImVec4 c);
        SeparatorPtr setText(const std::string &s);

        std::string getText() const;
    };
} // namespace mui