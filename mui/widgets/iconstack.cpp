// widgets/iconstack.cpp
#include "iconstack.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    IconStack::IconStack() { App::assertMainThread(); }

    void IconStack::renderControl()
    {
        if (!visible || icons.empty())
            return;
        ScopedID id(this);

        ImGui::BeginGroup();
        for (size_t i = 0; i < icons.size(); ++i)
        {
            ScopedID icon_id((int)i);
            if (ImGui::Button(icons[i].icon.c_str(), ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
            {
                onIconClickedSignal((int)i);
            }
            if (!icons[i].tooltip.empty() && ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", icons[i].tooltip.c_str());
            }
            if (i < icons.size() - 1)
                ImGui::SameLine();
        }
        ImGui::EndGroup();
    }

    IconStackPtr IconStack::add(const std::string &icon, std::function<void()> onClick, const std::string &tip)
    {
        int index = (int)icons.size();
        icons.push_back({icon, tip});

        if (onClick) {
            m_connections.push_back(onIconClickedSignal.connect([index, cb = std::move(onClick)](int clickedIndex) {
                if (clickedIndex == index) cb();
            }));
        }
        
        return self();
    }
}
