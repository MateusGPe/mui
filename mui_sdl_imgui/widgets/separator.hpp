// widgets/separator.hpp
#pragma once
#include "control.hpp"
#include <memory>

namespace mui
{
    class Separator;
    using SeparatorPtr = std::shared_ptr<Separator>;

    class Separator : public Control
    {
    protected:
        SeparatorPtr self() { return std::static_pointer_cast<Separator>(shared_from_this()); }

    public:
        static SeparatorPtr create() { return std::make_shared<Separator>(); }
        void renderControl() override;
    };
} // namespace mui