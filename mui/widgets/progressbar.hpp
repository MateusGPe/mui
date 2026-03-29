#pragma once
#include "control.hpp"
#include <string>
#include <memory>

namespace mui
{
    class ProgressBar;
    using ProgressBarPtr = std::shared_ptr<ProgressBar>;

    class ProgressBar : public Control<ProgressBar>
    {
    protected:
        std::string getTypeName() const override { return "ProgressBar"; }
        float value;
        std::string overlayText;
        ProgressBar();

    public:
        static ProgressBarPtr create() { return std::shared_ptr<ProgressBar>(new ProgressBar()); }

        void renderControl() override;

        ProgressBarPtr setValue(float v);
        ProgressBarPtr setOverlayText(const std::string &text);
        ProgressBarPtr setSpanAvailWidth(bool span);
        ProgressBarPtr setUseContainerWidth(bool use);
    };
} // namespace mui
