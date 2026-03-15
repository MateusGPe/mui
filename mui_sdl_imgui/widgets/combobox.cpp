// widgets/combobox.cpp
#include "combobox.hpp"
#include "../core/app.hpp"
#include <imgui.h>

namespace mui
{
    ComboBox::ComboBox() : selectedIndex(-1) { App::assertMainThread(); }

    void ComboBox::render()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        std::string preview = (selectedIndex >= 0 && selectedIndex < (int)items.size()) ? items[selectedIndex] : "";
        
        if (useContainerWidth)
        {
            ImGui::SetNextItemWidth(ImGui::CalcItemWidth());
        }

        if (ImGui::BeginCombo("##combo", preview.c_str())) {
            for (int i = 0; i < (int)items.size(); ++i) {
                const bool isSelected = (selectedIndex == i);
                if (ImGui::Selectable(items[i].c_str(), isSelected)) {
                    selectedIndex = i;
                    if (onSelectedCb) onSelectedCb();
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        
        renderTooltip();

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    ComboBoxPtr ComboBox::append(const std::string& item) {
        items.push_back(item);
        if (selectedIndex == -1) selectedIndex = 0;
        return self();
    }

    ComboBoxPtr ComboBox::clear() {
        items.clear();
        selectedIndex = -1;
        return self();
    }

    int ComboBox::getSelected() const { return selectedIndex; }
    
    ComboBoxPtr ComboBox::setSelected(int index) {
        if (index >= -1 && index < (int)items.size()) {
            selectedIndex = index;
        }
        return self();
    }

    ComboBoxPtr ComboBox::onSelected(std::function<void()> cb) {
        onSelectedCb = std::move(cb);
        return self();
    }

    ComboBoxPtr ComboBox::setUseContainerWidth(bool use) {
        useContainerWidth = use;
        return self();
    }
} // namespace mui