#pragma once
#include "../widgets/control.hpp"
#include <vector>
#include <memory>

namespace mui
{
    class Box;
    using BoxPtr = std::shared_ptr<Box>;

    class Box : public Control
    {
    protected:
        struct BoxChild
        {
            ControlPtr control;
            bool stretchy;
        };
        std::vector<BoxChild> children;
        bool padded = true;
        bool scrollable = false;
        bool autoScroll = false;

        BoxPtr self() { return std::static_pointer_cast<Box>(shared_from_this()); }

    public:
        Box();

        void onHandleDestroyed() override;

        BoxPtr append(ControlPtr child, bool stretchy = false);
        BoxPtr deleteChild(int index);

        BoxPtr setPadded(bool p);
        BoxPtr setScrollable(bool s);
        BoxPtr setAutoScroll(bool a);

        int getNumChildren() const;
        bool getPadded() const;
        bool getScrollable() const;
        bool getAutoScroll() const;
    };

    class VBox;
    using VBoxPtr = std::shared_ptr<VBox>;

    class VBox : public Box
    {
    protected:
        VBoxPtr self() { return std::static_pointer_cast<VBox>(shared_from_this()); }

    public:
        VBox();
        static VBoxPtr create() { return std::make_shared<VBox>(); }
        void render() override;
    };

    class HBox;
    using HBoxPtr = std::shared_ptr<HBox>;

    class HBox : public Box
    {
    protected:
        HBoxPtr self() { return std::static_pointer_cast<HBox>(shared_from_this()); }

    public:
        HBox();
        static HBoxPtr create() { return std::make_shared<HBox>(); }
        void render() override;
    };
} // namespace mui
