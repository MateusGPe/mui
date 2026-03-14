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
        struct Child
        {
            ControlPtr control;
            bool stretchy;
        };
        std::vector<Child> children;
        bool padded = false;
        bool scrollable = false;
        bool autoScroll = false;

    public:
        Box();
        BoxPtr self() { return std::static_pointer_cast<Box>(shared_from_this()); }

        void onHandleDestroyed();
        BoxPtr append(ControlPtr child, bool stretchy = false);
        BoxPtr deleteChild(int index);
        BoxPtr setPadded(bool padded);
        int getNumChildren() const;
        bool getPadded() const;
        BoxPtr setScrollable(bool s);
        BoxPtr setAutoScroll(bool a);
        bool getScrollable() const;
        bool getAutoScroll() const;
    };

    class VBox;
    using VBoxPtr = std::shared_ptr<VBox>;

    class VBox : public Box
    {
    public:
        VBox();
        static VBoxPtr create() { return std::make_shared<VBox>(); }
        void render() override;
    };

    class HBox;
    using HBoxPtr = std::shared_ptr<HBox>;

    class HBox : public Box
    {
    public:
        HBox();
        static HBoxPtr create() { return std::make_shared<HBox>(); }
        void render() override;
    };
} // namespace mui
