#include "tab.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    Tab::Tab() { 
        App::assertMainThread();
        m_flags = ImGuiTabBarFlags_TabListPopupButton;
    }

    void Tab::renderControl()
    {
        if (!visible) return;
        ScopedControlID id(this);
        if (ImGui::BeginTabBar("##tabs", m_flags)) {
            for (size_t i = 0; i < pages.size(); ++i) {
                if (ImGui::BeginTabItem(pages[i].name.c_str())) {
                    if (selectedIndex != (int)i) {
                        selectedIndex = (int)i;
                        onSelectedSignal(selectedIndex);
                    }

                    bool needsIndexId = pages[i].control->getID().empty();
                    if (needsIndexId) ImGui::PushID(static_cast<int>(i));

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

                    if (needsIndexId) ImGui::PopID();

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
    
    TabPtr Tab::onSelected(std::function<void(int)> cb)
    {
        if (cb) m_connections.push_back(onSelectedSignal.connect(std::move(cb)));
        return self();
    }
    int Tab::getNumPages() const { return pages.size(); }
    int Tab::getSelected() const { return selectedIndex; }

    TabPtr Tab::setSelected(int index)
    {
        if (index >= 0 && index < (int)pages.size())
            selectedIndex = index;
        return self();
    }

    TabPtr Tab::bindSelected(std::shared_ptr<Observable<int>> observable)
    {
        setSelected(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const int &val) { mui::App::queueMain([this, val]() { this->setSelected(val); }); }));
        m_connections.push_back(onSelectedSignal.connect([observable](int val) { observable->set(val); }));
        return self();
    }

    TabPtr Tab::setReorderable(bool b)
    {
        if (b) m_flags |= ImGuiTabBarFlags_Reorderable;
        else m_flags &= ~ImGuiTabBarFlags_Reorderable;
        return self();
    }

    TabPtr Tab::setAutoSelectNewTabs(bool b)
    {
        if (b) m_flags |= ImGuiTabBarFlags_AutoSelectNewTabs;
        else m_flags &= ~ImGuiTabBarFlags_AutoSelectNewTabs;
        return self();
    }

    TabPtr Tab::setNoCloseWithMiddleMouseButton(bool b)
    {
        if (b) m_flags |= ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
        else m_flags &= ~ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
        return self();
    }

    TabPtr Tab::setNoTabListScrollingButtons(bool b)
    {
        if (b) m_flags |= ImGuiTabBarFlags_NoTabListScrollingButtons;
        else m_flags &= ~ImGuiTabBarFlags_NoTabListScrollingButtons;
        return self();
    }

    TabPtr Tab::setNoTooltip(bool b)
    {
        if (b) m_flags |= ImGuiTabBarFlags_NoTooltip;
        else m_flags &= ~ImGuiTabBarFlags_NoTooltip;
        return self();
    }

    TabPtr Tab::setFittingPolicyScroll(bool b)
    {
        if (b) m_flags |= ImGuiTabBarFlags_FittingPolicyScroll;
        else m_flags &= ~ImGuiTabBarFlags_FittingPolicyScroll;
        return self();
    }
    TabPtr Tab::setFittingPolicyResizeDown(bool b)
    {
        if (b) m_flags |= ImGuiTabBarFlags_FittingPolicyResizeDown;
        else m_flags &= ~ImGuiTabBarFlags_FittingPolicyResizeDown;
        return self();
    }
} // namespace mui
