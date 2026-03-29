// widgets/coloredit.hpp
#pragma once
#include "control.hpp"
#include <array>
#include <functional>
#include <memory>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class ColorEdit;
    using ColorEditPtr = std::shared_ptr<ColorEdit>;

    class ColorEdit : public Control<ColorEdit>
    {
    protected:
        std::string getTypeName() const override { return "ColorEdit"; }
        float color[4];

        ColorEdit(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

    public:
        mui::Signal<std::array<float, 4>> onChangedSignal;

        static ColorEditPtr create(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        {
            return std::shared_ptr<ColorEdit>(new ColorEdit(r, g, b, a));
        }

        void renderControl() override;

        std::array<float, 4> getColor() const;
        ColorEditPtr setColor(float r, float g, float b, float a = 1.0f);
        ColorEditPtr bind(std::shared_ptr<Observable<std::array<float, 4>>> observable);
        ColorEditPtr onChanged(std::function<void(const std::array<float, 4> &)> cb);
    };
} // namespace mui