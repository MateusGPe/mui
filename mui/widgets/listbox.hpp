// widgets/listbox.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class ListBox;
    using ListBoxPtr = std::shared_ptr<ListBox>;

    class ListBox : public Control<ListBox>
    {
    protected:
        std::vector<std::string> items;
        int selectedIndex;
        int visibleItemsCount;
        ListBox();

    public:
        mui::Signal<int> onSelectedSignal;
        mui::Signal<int> onDoubleClickSignal;

        static ListBoxPtr create() { return std::shared_ptr<ListBox>(new ListBox()); }

        void renderControl() override;

        ListBoxPtr append(const std::string &item);
        ListBoxPtr clear();

        int getSelected() const;
        ListBoxPtr setSelected(int index);
        ListBoxPtr setVisibleItems(int count);
        ListBoxPtr bind(std::shared_ptr<Observable<int>> observable);
        ListBoxPtr setSpanAvailWidth(bool span);
        ListBoxPtr setUseContainerWidth(bool use);

        ListBoxPtr onSelected(std::function<void()> cb);
        ListBoxPtr onDoubleClick(std::function<void()> cb);
    };
} // namespace mui