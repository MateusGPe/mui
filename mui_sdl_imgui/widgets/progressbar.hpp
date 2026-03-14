#pragma once
#include "control.hpp"

namespace mui
{
    class ProgressBar;
    using ProgressBarPtr = std::shared_ptr<ProgressBar>;

    class ProgressBar : public Control
    {
        int value;

    public:
        ProgressBar();
        static ProgressBarPtr create() { return std::make_shared<ProgressBar>(); }
        ProgressBarPtr self() { return std::static_pointer_cast<ProgressBar>(shared_from_this()); }

        void render() override;
        ProgressBarPtr setValue(int val);
    };
} // namespace mui
