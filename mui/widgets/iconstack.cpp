// widgets/iconstack.cpp
#include "iconstack.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    IconStack::IconStack() { App::assertMainThread(); }

    void IconStack::renderControl()
    {
        if (!visible || icons.empty()) return;
        ImGui::PushID(this);
        
        ImGui::BeginGroup();
        for (size_t i = 0; i < icons.size(); ++i) {
            ImGui::PushID((int)i);
            if (ImGui::Button(icons[i].icon.c_str(), ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()))) {
                if (icons[i].cb) icons[i].cb();
            }
            if (!icons[i].tooltip.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", icons[i].tooltip.c_str());
            }
            if (i < icons.size() - 1) ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::EndGroup();
        
        ImGui::PopID();
    }

    IconStackPtr IconStack::add(const std::string& icon, std::function<void()> onClick, const std::string& tip) {
        icons.push_back({icon, tip, std::move(onClick)});
        return self();
    }
}
