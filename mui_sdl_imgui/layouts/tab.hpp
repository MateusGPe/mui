#pragma once
#include "../widgets/control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mui
{
    class Tab;
    using TabPtr = std::shared_ptr<Tab>;

    class Tab : public Control
    {
    protected:
        struct TabPage
        {
            std::string name;
            ControlPtr control;
        };
        std::vector<TabPage> pages;
        int selectedIndex = 0;
        std::function<void()> onSelectedCb;

        TabPtr self() { return std::static_pointer_cast<Tab>(shared_from_this()); }

    public:
        Tab();
        static TabPtr create() { return std::make_shared<Tab>(); }

        void renderControl() override;

        TabPtr append(const std::string &name, ControlPtr child);
        TabPtr setMargined(int page, bool margined) { return self(); } // Stub
        TabPtr onSelected(std::function<void()> cb);
        int getNumPages() const;
        int getSelected() const;
    };
} // namespace mui
