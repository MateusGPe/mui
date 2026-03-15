#include "control.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Control::~Control() {}

    void Control::onHandleDestroyed() { ownsHandle = false; }
    void Control::verifyState() const { App::assertMainThread(); }

    void Control::show() { visible = true; }
    void Control::hide() { visible = false; }

    void Control::setEnabled(bool enabled) { this->enabled = enabled; }
    bool Control::isEnabled() const { return enabled; }

    void Control::releaseOwnership() { ownsHandle = false; }
    void Control::acquireOwnership() { ownsHandle = true; }

    ControlPtr Control::setTooltip(const std::string &t)
    {
        tooltip = t;
        return shared_from_this();
    }
    void Control::renderTooltip()
    {
        if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        {
            ImGui::SetTooltip("%s", tooltip.c_str());
        }
    }
}