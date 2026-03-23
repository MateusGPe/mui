// layouts/splitter.hpp
#pragma once
#include "../widgets/control.hpp"
#include <memory>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class SplitterView;
    using SplitterViewPtr = std::shared_ptr<SplitterView>;

    enum class SplitterOrientation
    {
        Horizontal,
        Vertical
    };

    class SplitterView : public Control<SplitterView>
    {
    protected:
        IControlPtr panel1;
        IControlPtr panel2;
        SplitterOrientation orientation;
        float splitRatio = 0.25f; // Default 25% / 75%
        float thickness = 4.0f;

        SplitterView(SplitterOrientation o = SplitterOrientation::Horizontal);

    public:
        mui::Signal<float> onSplitRatioChangedSignal;

        static SplitterViewPtr create(SplitterOrientation o = SplitterOrientation::Horizontal)
        {
            return std::shared_ptr<SplitterView>(new SplitterView(o));
        }

        void renderControl() override;

        SplitterViewPtr setPanel1(IControlPtr c);
        SplitterViewPtr setPanel2(IControlPtr c);
        SplitterViewPtr setSplitRatio(float ratio);
        SplitterViewPtr setThickness(float t);

        float getSplitRatio() const;
        SplitterViewPtr onSplitRatioChanged(std::function<void(float)> cb);
        SplitterViewPtr bindSplitRatio(std::shared_ptr<Observable<float>> observable);
    };
} // namespace mui
