#pragma once
#include "control.hpp"
#include <memory>

namespace mui
{
    class Card;
    using CardPtr = std::shared_ptr<Card>;

    class Card : public Control
    {
    protected:
        ControlPtr child;
        float padding = 8.0f;
        float shadowBlur = 0.0f;
        float shadowOffset = 0.0f;
        int shadowAlpha = 0;

        CardPtr self() { return std::static_pointer_cast<Card>(shared_from_this()); }

    public:
        Card();
        static CardPtr create() { return std::make_shared<Card>(); }

        void render() override;

        CardPtr setChild(ControlPtr c);
        CardPtr setPadding(float p);
        CardPtr setShadow(float blur, float offset, int alpha);
    };
} // namespace mui