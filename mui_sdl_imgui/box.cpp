#include "box.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Box::Box() { App::assertMainThread(); }

    void Box::onHandleDestroyed()
    {
        Control::onHandleDestroyed();
        for (auto &c : children) c->onHandleDestroyed();
    }

    BoxPtr Box::append(ControlPtr child, bool stretchy)
    {
        verifyState();
        children.push_back(child);
        return self();
    }

    BoxPtr Box::deleteChild(int index)
    {
        verifyState();
        if (index >= 0 && index < children.size()) {
            children.erase(children.begin() + index);
        }
        return self();
    }

    BoxPtr Box::setPadded(bool p) { padded = p; return self(); }
    int Box::getNumChildren() const { return children.size(); }
    bool Box::getPadded() const { return padded; }

    VBox::VBox() : Box() {}
    void VBox::render() {
        if (!visible) return;
        ImGui::PushID(this);
        for (auto& child : children) {
            child->render();
            if (padded) ImGui::Spacing();
        }
        ImGui::PopID();
    }

    HBox::HBox() : Box() {}
    void HBox::render() {
        if (!visible) return;
        ImGui::PushID(this);
        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->render();
            if (i < children.size() - 1) ImGui::SameLine();
        }
        ImGui::PopID();
    }
} // namespace mui
