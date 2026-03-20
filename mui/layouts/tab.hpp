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

    class Tab : public Control<Tab>
    {
    protected:
        struct TabPage
        {
            std::string name;
            IControlPtr control;
            bool margined = false;
        };
        std::vector<TabPage> pages;
        int selectedIndex = 0;
        std::function<void()> onSelectedCb;

    public:
        Tab();
        static TabPtr create() { return std::make_shared<Tab>(); }

        void renderControl() override;

        TabPtr append(const std::string &name, IControlPtr child);
        TabPtr setMargined(int page, bool margined)
        {
            if (page >= 0 && page < pages.size())
            {
                pages[page].margined = margined;
            }
            return self();
        }
        TabPtr onSelected(std::function<void()> cb);
        int getNumPages() const;
        int getSelected() const;
    };
} // namespace mui
