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
        ImGuiTabBarFlags m_flags;

        Tab();

    public:
        mui::Signal<int> onSelectedSignal;

        static TabPtr create() { return std::shared_ptr<Tab>(new Tab()); }

        void renderControl() override;

        TabPtr append(const std::string &name, IControlPtr child);
        TabPtr setMargined(int page, bool margined)
        {
            std::size_t pageCount(page);
            if (page < 0 || pageCount >= pages.size())
                return self();

            pages[pageCount].margined = margined;
            return self();
        }
        TabPtr onSelected(std::function<void(int)> cb);
        int getNumPages() const;
        int getSelected() const;

        TabPtr setSelected(int index);
        TabPtr bindSelected(std::shared_ptr<Observable<int>> observable);

        TabPtr setReorderable(bool b);
        TabPtr setAutoSelectNewTabs(bool b);
        TabPtr setNoCloseWithMiddleMouseButton(bool b);
        TabPtr setNoTabListScrollingButtons(bool b);
        TabPtr setNoTooltip(bool b);
        TabPtr setFittingPolicyScroll(bool b);
        TabPtr setFittingPolicyResizeDown(bool b);
    };
} // namespace mui