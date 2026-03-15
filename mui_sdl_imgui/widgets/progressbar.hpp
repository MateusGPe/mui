#pragma once
#include "control.hpp"
#include <string>
#include <memory>

namespace mui
{
    class ProgressBar;
    using ProgressBarPtr = std::shared_ptr<ProgressBar>;

    class ProgressBar : public Control
    {
    protected:
        float value;
        std::string overlayText;

        ProgressBarPtr self() { return std::static_pointer_cast<ProgressBar>(shared_from_this()); }

    public:
        ProgressBar();
        static ProgressBarPtr create() { return std::make_shared<ProgressBar>(); }

        void renderControl() override;

        ProgressBarPtr setValue(float v);
        ProgressBarPtr setOverlayText(const std::string &text);
        ProgressBarPtr setSpanAvailWidth(bool span);
    };
} // namespace mui
