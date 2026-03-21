#pragma once
#include "../widgets/control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

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

    public:
        mui::Signal<int> onSelectedSignal;

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
        TabPtr onSelected(std::function<void(int)> cb);
        int getNumPages() const;
        int getSelected() const;

        TabPtr setSelected(int index);
        TabPtr bindSelected(std::shared_ptr<Observable<int>> observable);
    };
} // namespace mui
