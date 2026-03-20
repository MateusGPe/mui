// layouts/splitter.hpp
#pragma once
#include "../widgets/control.hpp"
#include <memory>

namespace mui
{
    class SplitterView;
    using SplitterViewPtr = std::shared_ptr<SplitterView>;

    enum class SplitterOrientation
    {
        Horizontal,
        Vertical
    };

    class SplitterView : public Control
    {
    protected:
        ControlPtr panel1;
        ControlPtr panel2;
        SplitterOrientation orientation;
        float splitRatio = 0.25f; // Default 25% / 75%
        float thickness = 4.0f;

        SplitterViewPtr self() { return std::static_pointer_cast<SplitterView>(shared_from_this()); }

    public:
        SplitterView(SplitterOrientation o = SplitterOrientation::Horizontal);
        static SplitterViewPtr create(SplitterOrientation o = SplitterOrientation::Horizontal) { return std::make_shared<SplitterView>(o); }

        void renderControl() override;

        SplitterViewPtr setPanel1(ControlPtr c);
        SplitterViewPtr setPanel2(ControlPtr c);
        SplitterViewPtr setSplitRatio(float ratio);
        SplitterViewPtr setThickness(float t);
    };
} // namespace mui
