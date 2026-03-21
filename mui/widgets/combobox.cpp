// widgets/combobox.cpp
#include "combobox.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    ComboBox::ComboBox() : selectedIndex(-1) { App::assertMainThread(); }

    void ComboBox::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        std::string preview = (selectedIndex >= 0 && selectedIndex < (int)items.size()) ? items[selectedIndex] : "";

        auto render_combo = [&]
        {
            if (ImGui::BeginCombo("##combo", preview.c_str()))
            {
                for (int i = 0; i < (int)items.size(); ++i)
                {
                    const bool isSelected = (selectedIndex == i);
                    if (ImGui::Selectable(items[i].c_str(), isSelected))
                    {
                        selectedIndex = i;
                        if (onChangedCb)
                            onChangedCb();
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        };

        {
            ScopedItemWidth width;
            if (spanAvailWidth)
            {
                width.push(-FLT_MIN);
            }
            else if (useContainerWidth)
            {
                width.push(ImGui::CalcItemWidth());
            }
            render_combo();
        }

        renderTooltip();

        ImGui::EndDisabled();
    }

    ComboBoxPtr ComboBox::append(const std::string &item)
    {
        items.push_back(item);
        if (selectedIndex == -1)
            selectedIndex = 0;
        return self();
    }

    ComboBoxPtr ComboBox::clear()
    {
        items.clear();
        selectedIndex = -1;
        return self();
    }

    int ComboBox::getSelectedIndex() const { return selectedIndex; }

    std::string ComboBox::getText() const
    {
        if (selectedIndex >= 0 && selectedIndex < (int)items.size())
        {
            return items[selectedIndex];
        }
        return "";
    }

    ComboBoxPtr ComboBox::setSelectedIndex(int index)
    {
        if (index >= -1 && index < (int)items.size())
        {
            selectedIndex = index;
        }
        return self();
    }

    ComboBoxPtr ComboBox::onChanged(std::function<void()> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }

    ComboBoxPtr ComboBox::setSpanAvailWidth(bool span)
    {
        spanAvailWidth = span;
        return self();
    }
} // namespace mui