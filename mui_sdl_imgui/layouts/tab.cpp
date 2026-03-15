#include "tab.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Tab::Tab() { App::assertMainThread(); }

    void Tab::renderControl()
    {
        if (!visible) return;
        ImGui::PushID(this);
        if (ImGui::BeginTabBar("##tabs")) {
            for (size_t i = 0; i < pages.size(); ++i) {
                if (ImGui::BeginTabItem(pages[i].name.c_str())) {
                    if (selectedIndex != i) {
                        selectedIndex = i;
                        if (onSelectedCb) onSelectedCb();
                    }
                    pages[i].control->render();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::PopID();
    }

    TabPtr Tab::append(const std::string &name, ControlPtr child)
    {
        pages.push_back({name, child});
        return self();
    }
    
    TabPtr Tab::onSelected(std::function<void()> cb) { onSelectedCb = std::move(cb); return self(); }
    int Tab::getNumPages() const { return pages.size(); }
    int Tab::getSelected() const { return selectedIndex; }
} // namespace mui
