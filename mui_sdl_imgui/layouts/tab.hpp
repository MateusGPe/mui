#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>

namespace mui
{
    class Tab;
    using TabPtr = std::shared_ptr<Tab>;

    class Tab : public Control
    {
        struct Page {
            std::string name;
            ControlPtr control;
        };
        std::vector<Page> pages;
        int selectedIndex = 0;
        std::function<void()> onSelectedCb;

    public:
        Tab();
        TabPtr self() { return std::static_pointer_cast<Tab>(shared_from_this()); }

        static TabPtr create() { return std::make_shared<Tab>(); }
        void render() override;
        TabPtr append(const std::string &name, ControlPtr child);
        TabPtr setMargined(int page, bool margined) { return self(); } // Stub
        TabPtr onSelected(std::function<void()> cb);
        int getNumPages() const;
        int getSelected() const;
    };
} // namespace mui
