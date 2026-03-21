// widgets/listbox.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

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
        std::function<void()> onSelectedCb;
        std::function<void()> onDoubleClickCb;

    public:
        ListBox();
        static ListBoxPtr create() { return std::make_shared<ListBox>(); }

        void renderControl() override;

        ListBoxPtr append(const std::string &item);
        ListBoxPtr clear();

        int getSelected() const;
        ListBoxPtr setSelected(int index);
        ListBoxPtr setVisibleItems(int count);
        ListBoxPtr setSpanAvailWidth(bool span);
        ListBoxPtr setUseContainerWidth(bool use);

        ListBoxPtr onSelected(std::function<void()> cb);
        ListBoxPtr onDoubleClick(std::function<void()> cb);
    };
} // namespace mui