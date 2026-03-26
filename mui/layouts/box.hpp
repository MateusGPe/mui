#pragma once
#include "../widgets/control.hpp"
#include <vector>
#include <memory>

namespace mui
{
    template <class Derived>
    class Box : public Control<Derived>
    {
    public:
        struct BoxChild
        {
            IControlPtr control;
            bool stretchy;

            BoxChild(IControlPtr c, bool s = false) : control(std::move(c)), stretchy(s) {}
        };

    protected:
        std::vector<BoxChild> children;
        bool padded = true;
        bool scrollable = false;
        bool autoScroll = false;

        Box() { this->verifyState(); }

    public:
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

        std::shared_ptr<Derived> append(std::initializer_list<BoxChild> items)
        {
            this->verifyState();
            for (const auto &item : items)
            {
                children.push_back(item);
            }
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
    protected:
        VBox();
    public:
        static VBoxPtr create() { return std::shared_ptr<VBox>(new VBox()); }
        void renderControl() override;
    };

    class HBox;
    using HBoxPtr = std::shared_ptr<HBox>;

    class HBox : public Box<HBox>
    {
    protected:
        HBox();
    public:
        static HBoxPtr create() { return std::shared_ptr<HBox>(new HBox()); }
        void renderControl() override;
    };

    class FlowBox;
    using FlowBoxPtr = std::shared_ptr<FlowBox>;

    class FlowBox : public Box<FlowBox>
    {
    public:
        enum class Align
        {
            Left,
            Center,
            Right,
            Justify
        };

    protected:
        struct flow_internal
        {
            float lastpos;
            float lastline;
            float lastKnownHeight;
            float lastKnownWidth;
        };
        std::vector<flow_internal> flow_data;
        Align m_align = Align::Left;

        FlowBox();

    public:
        static FlowBoxPtr create() { return std::shared_ptr<FlowBox>(new FlowBox()); }
        void renderControl() override;
        FlowBoxPtr append(IControlPtr child, bool stretchy = false);
        FlowBoxPtr append(std::initializer_list<BoxChild> items)
        {
            this->verifyState();
            for (const auto &item : items)
            {
                children.push_back(item);
                flow_data.push_back({0, 0, 0, 0});
            }
            return this->self();
        }
        FlowBoxPtr setAlign(Align align)
        {
            m_align = align;
            return self();
        }
    };
} // namespace mui
