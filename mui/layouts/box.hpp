#pragma once
#include "../widgets/control.hpp"
#include <vector>
#include <memory>

namespace mui
{
    template <class Derived>
    class Box : public Control<Derived>
    {
    protected:
        struct BoxChild
        {
            IControlPtr control;
            bool stretchy;
        };
        std::vector<BoxChild> children;
        bool padded = true;
        bool scrollable = false;
        bool autoScroll = false;

    public:
        Box() { this->verifyState(); }

        void onHandleDestroyed() override
        {
            Control<Derived>::onHandleDestroyed();
            for (auto &child : children)
                child.control->onHandleDestroyed();
        }

        std::shared_ptr<Derived> append(IControlPtr child, bool stretchy = false)
        {
            this->verifyState();
            children.push_back({child, stretchy});
            return this->self();
        }

        std::shared_ptr<Derived> deleteChild(int index)
        {
            this->verifyState();
            if (index >= 0 && index < (int)children.size())
            {
                children.erase(this->children.begin() + index);
            }
            return this->self();
        }

        std::shared_ptr<Derived> setPadded(bool p)
        {
            padded = p;
            return this->self();
        }

        std::shared_ptr<Derived> setScrollable(bool s)
        {
            scrollable = s;
            return this->self();
        }

        std::shared_ptr<Derived> setAutoScroll(bool a)
        {
            autoScroll = a;
            return this->self();
        }

        int getNumChildren() const { return this->children.size(); }
        bool getPadded() const { return padded; }
        bool getScrollable() const { return scrollable; }
        bool getAutoScroll() const { return autoScroll; }
    };

    class VBox;
    using VBoxPtr = std::shared_ptr<VBox>;

    class VBox : public Box<VBox>
    {
    public:
        VBox();
        static VBoxPtr create() { return std::make_shared<VBox>(); }
        void renderControl() override;
    };

    class HBox;
    using HBoxPtr = std::shared_ptr<HBox>;

    class HBox : public Box<HBox>
    {
    public:
        HBox();
        static HBoxPtr create() { return std::make_shared<HBox>(); }
        void renderControl() override;
    };
} // namespace mui

