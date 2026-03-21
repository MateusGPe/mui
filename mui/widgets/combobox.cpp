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

        float w = width;
        if (spanAvailWidth)
            w = -FLT_MIN;

        ScopedItemWidth item_width;
        if (w != 0.0f)
            item_width.push(w);
        else if (minSize.x > 0)
            ImGui::SetNextItemWidth(minSize.x);
        // useContainerWidth is handled by not pushing a width.

        if (ImGui::BeginCombo("##combo", preview.c_str()))
        {
            for (int i = 0; i < (int)items.size(); ++i)
            {
                const bool isSelected = (selectedIndex == i);
                if (ImGui::Selectable(items[i].c_str(), isSelected))
                {
                    selectedIndex = i;
                    onChangedSignal(i);
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
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

    ComboBoxPtr ComboBox::bind(std::shared_ptr<Observable<int>> observable)
    {
        setSelectedIndex(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const int &val)
                                                                   { mui::App::queueMain([this, val]()
                                                                                         { this->setSelectedIndex(val); }); }));
        m_connections.push_back(onChangedSignal.connect([observable](int val)
                                                        { observable->set(val); }));
        return self();
    }

    ComboBoxPtr ComboBox::onChanged(std::function<void()> cb)
    {
        if (cb)
            m_connections.push_back(onChangedSignal.connect([cb](int)
                                                            { cb(); }));
        return self();
    }

    ComboBoxPtr ComboBox::setSpanAvailWidth(bool span)
    {
        spanAvailWidth = span;
        return self();
    }

    ComboBoxPtr ComboBox::setMinWidth(float w)
    {
        minSize.x = w;
        return self();
    }
} // namespace mui