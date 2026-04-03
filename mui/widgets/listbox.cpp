// widgets/listbox.cpp
#include "listbox.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    ListBox::ListBox() : selectedIndex(-1), visibleItemsCount(5)
    {
        App::assertMainThread();
    }

    void ListBox::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImVec2 final_size = ApplySizeConstraints(ImVec2(0.0f, visibleItemsCount * ImGui::GetTextLineHeightWithSpacing()));

        if (ImGui::BeginListBox("##listbox", final_size))
        {
            for (int i = 0; i < (int)items.size(); ++i)
            {
                ScopedID itemId(i);
                const bool isSelected = (selectedIndex == i);
                if (ImGui::Selectable(items[i].c_str(), isSelected))
                {
                    selectedIndex = i;
                    onSelectedSignal(selectedIndex);
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    onDoubleClickSignal(i);
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }

        renderTooltip();

        ImGui::EndDisabled();
    }

    ListBoxPtr ListBox::append(const std::string &item)
    {
        items.push_back(item);
        return self();
    }
    ListBoxPtr ListBox::clear()
    {
        items.clear();
        selectedIndex = -1;
        return self();
    }
    int ListBox::getSelected() const { return selectedIndex; }
    ListBoxPtr ListBox::setSelected(int index)
    {
        selectedIndex = index;
        return self();
    }

    ListBoxPtr ListBox::bind(std::shared_ptr<Observable<int>> observable)
    {
        setSelected(observable->get());
        m_connections.push_back(
            observable->onValueChanged.connect([this](const int &val)
                                               { mui::App::queueMain([this, val]()
                                                                     { this->setSelected(val); }); }));
        m_connections.push_back(onSelectedSignal.connect(
            [observable](int val)
            { observable->set(val); }));
        return self();
    }

    ListBoxPtr ListBox::setVisibleItems(int count)
    {
        visibleItemsCount = count;
        return self();
    }
    ListBoxPtr ListBox::setSpanAvailWidth(bool span)
    {
        spanAvailWidth = span;
        return self();
    }
    ListBoxPtr ListBox::onSelected(std::function<void()> cb)
    {
        if (cb)
            m_connections.push_back(onSelectedSignal.connect([cb](int)
                                                             { cb(); }));
        return self();
    }
    ListBoxPtr ListBox::onDoubleClick(std::function<void()> cb)
    {
        if (cb)
            m_connections.push_back(onDoubleClickSignal.connect([cb](int)
                                                                { cb(); }));
        return self();
    }

    ListBoxPtr ListBox::setUseContainerWidth(bool use)
    {
        useContainerWidth = use;
        return self();
    }
} // namespace mui