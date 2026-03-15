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
        bool fillWidth = false;
        bool fillHeight = false;

        CardPtr self() { return std::static_pointer_cast<Card>(shared_from_this()); }

    public:
        Card();
        static CardPtr create() { return std::make_shared<Card>(); }

        void renderControl() override;

        CardPtr setChild(ControlPtr c);
        CardPtr setPadding(float p);
        CardPtr setFillWidth(bool fill);
        CardPtr setFillHeight(bool fill);
    };
} // namespace mui