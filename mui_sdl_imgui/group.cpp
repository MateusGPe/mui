#include "group.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Group::Group(const std::string &title) : title(title) { App::assertMainThread(); }

    void Group::render()
    {
        if (!visible) return;
        ImGui::PushID(this);
        if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            if (margined) ImGui::Indent();
            if (child) child->render();
            if (margined) ImGui::Unindent();
        }
        ImGui::PopID();
    }

    GroupPtr Group::setChild(ControlPtr c) { child = c; return self(); }
    GroupPtr Group::setMargined(bool m) { margined = m; return self(); }
    std::string Group::getTitle() const { return title; }
    GroupPtr Group::setTitle(const std::string &t) { title = t; return self(); }
    bool Group::getMargined() const { return margined; }
} // namespace mui
