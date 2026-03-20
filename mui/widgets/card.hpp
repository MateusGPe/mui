#pragma once
#include "control.hpp"
#include <memory>

namespace mui
{
    class Card;
    using CardPtr = std::shared_ptr<Card>;

    class Card : public Control<Card>
    {
    protected:
        IControlPtr child;
        float padding = 8.0f;
        bool fillHeight = false;

    public:
        Card();
        static CardPtr create() { return std::make_shared<Card>(); }

        void renderControl() override;

        CardPtr setChild(IControlPtr c);
        CardPtr setPadding(float p);
        CardPtr setFillHeight(bool fill);
    };
} // namespace mui