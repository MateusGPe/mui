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

    class ListBox : public Control
    {
    protected:
        std::vector<std::string> items;
        int selectedIndex;
        int visibleItemsCount;
        float width = 0.0f;
        std::function<void()> onSelectedCb;
        std::function<void()> onDoubleClickCb;

        ListBoxPtr self() { return std::static_pointer_cast<ListBox>(shared_from_this()); }

    public:
        ListBox();
        static ListBoxPtr create() { return std::make_shared<ListBox>(); }

        void renderControl() override;

        ListBoxPtr append(const std::string &item);
        ListBoxPtr clear();

        int getSelected() const;
        ListBoxPtr setSelected(int index);
        ListBoxPtr setVisibleItems(int count);
        ListBoxPtr setWidth(float w);

        ListBoxPtr onSelected(std::function<void()> cb);
        ListBoxPtr onDoubleClick(std::function<void()> cb);
    };
} // namespace mui