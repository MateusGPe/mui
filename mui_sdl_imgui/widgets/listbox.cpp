// widgets/listbox.cpp
#include "listbox.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    ListBox::ListBox() : selectedIndex(-1), visibleItemsCount(5) { App::assertMainThread(); }

    void ListBox::renderControl()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        if (width > 0) ImGui::SetNextItemWidth(width);

        if (ImGui::BeginListBox("##listbox", ImVec2(0, visibleItemsCount * ImGui::GetTextLineHeightWithSpacing()))) {
            for (int i = 0; i < (int)items.size(); ++i) {
                const bool isSelected = (selectedIndex == i);
                if (ImGui::Selectable(items[i].c_str(), isSelected)) {
                    selectedIndex = i;
                    if (onSelectedCb) onSelectedCb();
                }
                
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    if (onDoubleClickCb) onDoubleClickCb();
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }

        renderTooltip();

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    ListBoxPtr ListBox::append(const std::string& item) { items.push_back(item); return self(); }
    ListBoxPtr ListBox::clear() { items.clear(); selectedIndex = -1; return self(); }
    int ListBox::getSelected() const { return selectedIndex; }
    ListBoxPtr ListBox::setSelected(int index) { selectedIndex = index; return self(); }
    ListBoxPtr ListBox::setVisibleItems(int count) { visibleItemsCount = count; return self(); }
    ListBoxPtr ListBox::setWidth(float w) { width = w; return self(); }
    ListBoxPtr ListBox::onSelected(std::function<void()> cb) { onSelectedCb = std::move(cb); return self(); }
    ListBoxPtr ListBox::onDoubleClick(std::function<void()> cb) { onDoubleClickCb = std::move(cb); return self(); }
} // namespace mui