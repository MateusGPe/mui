// widgets/radiogroup.cpp
#include "radiogroup.hpp"
#include "../core/app.hpp"
#include <imgui.h>

namespace mui
{
    RadioGroup::RadioGroup() : selectedIndex(-1), horizontal(false) { App::assertMainThread(); }

    void RadioGroup::render()
    {
        if (!visible || options.empty()) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        for (int i = 0; i < (int)options.size(); ++i) {
            if (ImGui::RadioButton(options[i].c_str(), &selectedIndex, i)) {
                if (onSelectedCb) onSelectedCb();
            }
            if (horizontal && i < (int)options.size() - 1) {
                ImGui::SameLine();
            }
        }
        
        renderTooltip();

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    RadioGroupPtr RadioGroup::append(const std::string& option) {
        options.push_back(option);
        if (selectedIndex == -1) selectedIndex = 0;
        return self();
    }

    int RadioGroup::getSelected() const { return selectedIndex; }
    
    RadioGroupPtr RadioGroup::setSelected(int index) {
        if (index >= 0 && index < (int)options.size()) {
            selectedIndex = index;
        }
        return self();
    }

    RadioGroupPtr RadioGroup::setHorizontal(bool h) {
        horizontal = h;
        return self();
    }

    RadioGroupPtr RadioGroup::onSelected(std::function<void()> cb) {
        onSelectedCb = std::move(cb);
        return self();
    }
} // namespace mui