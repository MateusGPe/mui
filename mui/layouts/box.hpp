// layouts/box.hpp
#pragma once
#include "../widgets/control.hpp"
#include <vector>
#include <memory>

namespace mui
{
    struct Sizing
    {
        enum class Mode
        {
            Auto,    // Content-based size (default)
            Stretch, // Fill remaining space (weight)
            Fixed,   // Fixed pixels
            Percent  // Percentage of parent container
        };

        Mode mode = Mode::Auto;
        float value = 1.0f; // For Stretch, it's the weight. For Fixed/Percent, it's the value.

        Sizing(Mode m = Mode::Auto, float v = 1.0f) : mode(m), value(v) {}

        static Sizing Auto() { return {Mode::Auto, 0.0f}; }
        static Sizing Stretch(float weight = 1.0f) { return {Mode::Stretch, weight}; }
        static Sizing Fixed(float pixels) { return {Mode::Fixed, pixels}; }
        static Sizing Percent(float percent) { return {Mode::Percent, percent / 100.0f}; }
    };

    template <class Derived>
    class Box : public Control<Derived>
    {
    public:
        struct BoxChild
        {
            IControlPtr control;
            Sizing sizing;
            ImVec2 lastKnownSize = ImVec2(0, 0); // Added to cache layout sizes for Box stretching

            BoxChild(IControlPtr c, Sizing s = {}) : control(std::move(c)), sizing(s) {}
            BoxChild(IControlPtr c, bool stretchy) : control(std::move(c))
            {
                sizing = stretchy ? Sizing::Stretch() : Sizing::Auto();
            }
        };

    protected:
        std::vector<BoxChild> children;
        bool padded = true;
        bool scrollable = false;
        bool autoScroll = false;
        bool fillWidth = false;
        bool fillHeight = false;
        bool showChildScrollbars = false;
        float spacing = -1.0f; // -1 means default

        Box()
        {
            this->verifyState();
            this->inlineShadowEnabled = false;
        }

    public:
        void onHandleDestroyed() override
        {
            Control<Derived>::onHandleDestroyed();
            for (auto &child : children)
                child.control->onHandleDestroyed();
        }

        std::shared_ptr<Derived> append(IControlPtr child, Sizing sizing)
        {
            this->verifyState();
            children.emplace_back(child, sizing);
            return this->self();
        }

        std::shared_ptr<Derived> append(IControlPtr child, bool stretchy = false)
        {
            this->verifyState();
            children.emplace_back(child, stretchy);
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

        std::shared_ptr<Derived> setFillWidth(bool f)
        {
            fillWidth = f;
            return this->self();
        }

        std::shared_ptr<Derived> setFillHeight(bool f)
        {
            fillHeight = f;
            return this->self();
        }

        std::shared_ptr<Derived> setShowChildScrollbars(bool show)
        {
            showChildScrollbars = show;
            return this->self();
        }

        std::shared_ptr<Derived> setSpacing(float s)
        {
            spacing = s;
            return this->self();
        }

        int getNumChildren() const { return this->children.size(); }
        bool getPadded() const { return padded; }
        bool getScrollable() const { return scrollable; }
        bool getAutoScroll() const { return autoScroll; }
        bool getFillWidth() const { return fillWidth; }
        bool getFillHeight() const { return fillHeight; }
        bool getShowChildScrollbars() const { return showChildScrollbars; }
        float getSpacing() const { return spacing; }
    };

    class VBox;
    using VBoxPtr = std::shared_ptr<VBox>;

    class VBox : public Box<VBox>
    {
    protected:
        std::string getTypeName() const override { return "VBox"; }
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
        std::string getTypeName() const override { return "HBox"; }
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
        std::string getTypeName() const override { return "FlowBox"; }
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
        
        FlowBoxPtr setAlign(Align align)
        {
            m_align = align;
            return self();
        }
    };
} // namespace mui