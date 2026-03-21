#include "tab.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    Tab::Tab() { App::assertMainThread(); }

    void Tab::renderControl()
    {
        if (!visible) return;
        ScopedID id(this);
        if (ImGui::BeginTabBar("##tabs")) {
            for (size_t i = 0; i < pages.size(); ++i) {
                if (ImGui::BeginTabItem(pages[i].name.c_str())) {
                    if (selectedIndex != i) {
                        selectedIndex = i;
                        if (onSelectedCb) onSelectedCb();
                    }

                    if (pages[i].margined)
                    {
                        ScopedColor colors;
                        colors.push(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                        colors.push(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                        ImGui::BeginChild("##margined_tab_content", ImVec2(0, 0), ImGuiChildFlags_FrameStyle);
                        pages[i].control->render();
                        ImGui::EndChild();
                    }
                    else
                    {
                        pages[i].control->render();
                    }
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }

    TabPtr Tab::append(const std::string &name, IControlPtr child)
    {
        pages.push_back({name, child, false});
        return self();
    }
    
    TabPtr Tab::onSelected(std::function<void()> cb) { onSelectedCb = std::move(cb); return self(); }
    int Tab::getNumPages() const { return pages.size(); }
    int Tab::getSelected() const { return selectedIndex; }
} // namespace mui
