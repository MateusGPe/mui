#pragma once
#include "control.hpp"
#include <vector>

namespace mui
{
    class Box;
    using BoxPtr = std::shared_ptr<Box>;

    class Box : public Control
    {
    protected:
        std::vector<ControlPtr> children;
        bool padded = false;

    public:
        Box();
        BoxPtr self() { return std::static_pointer_cast<Box>(shared_from_this()); }

        void onHandleDestroyed();
        BoxPtr append(ControlPtr child, bool stretchy = false);
        BoxPtr deleteChild(int index);
        BoxPtr setPadded(bool padded);
        int getNumChildren() const;
        bool getPadded() const;
    };

    class VBox : public Box
    {
    public:
        VBox();
        void render() override;
    };

    class HBox : public Box
    {
    public:
        HBox();
        void render() override;
    };
} // namespace mui
