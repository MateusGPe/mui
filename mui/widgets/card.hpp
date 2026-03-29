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
        std::string getTypeName() const override { return "Card"; }
        IControlPtr child;
        float padding = 8.0f;
        bool fillHeight = false;

        Card();

    public:
        static CardPtr create() { return std::shared_ptr<Card>(new Card()); }

        void renderControl() override;

        CardPtr setChild(IControlPtr c);
        CardPtr setPadding(float p);
        CardPtr setFillHeight(bool fill);
    };
} // namespace mui