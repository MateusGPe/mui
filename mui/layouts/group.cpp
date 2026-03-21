#include "group.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    Group::Group(const std::string &title) : title(title) { App::assertMainThread(); }

    void Group::renderControl()
    {
        if (!visible) return;
        ScopedID id(this);
        if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            if (margined) ImGui::Indent();
            if (child) child->render();
            if (margined) ImGui::Unindent();
        }
    }

    GroupPtr Group::setChild(IControlPtr c) { child = c; return self(); }
    GroupPtr Group::setMargined(bool m) { margined = m; return self(); }
    std::string Group::getTitle() const { return title; }
    GroupPtr Group::setTitle(const std::string &t) { title = t; return self(); }
    bool Group::getMargined() const { return margined; }
} // namespace mui
