// widgets/coloredit.hpp
#pragma once
#include "control.hpp"
#include <array>
#include <functional>
#include <memory>

namespace mui
{
    class ColorEdit;
    using ColorEditPtr = std::shared_ptr<ColorEdit>;

    class ColorEdit : public Control
    {
    protected:
        float color[4];
        std::function<void()> onChangedCb;

        ColorEditPtr self() { return std::static_pointer_cast<ColorEdit>(shared_from_this()); }

    public:
        ColorEdit(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
        static ColorEditPtr create(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        {
            return std::make_shared<ColorEdit>(r, g, b, a);
        }

        void renderControl() override;

        std::array<float, 4> getColor() const;
        ColorEditPtr setColor(float r, float g, float b, float a = 1.0f);
        ColorEditPtr onChanged(std::function<void()> cb);
    };
} // namespace mui