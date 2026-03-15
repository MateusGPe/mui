#include "box.hpp"
#include "app.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <numeric>

namespace mui
{
    Box::Box() { App::assertMainThread(); }

    void Box::onHandleDestroyed()
    {
        Control::onHandleDestroyed();
        for (auto &child : children)
            child.control->onHandleDestroyed();
    }

    BoxPtr Box::append(ControlPtr child, bool stretchy)
    {
        verifyState();
        children.push_back({child, stretchy});
        return self();
    }

    BoxPtr Box::deleteChild(int index)
    {
        verifyState();
        if (index >= 0 && index < (int)children.size())
        {
            children.erase(children.begin() + index);
        }
        return self();
    }

    BoxPtr Box::setPadded(bool p)
    {
        padded = p;
        return self();
    }

    BoxPtr Box::setScrollable(bool s)
    {
        scrollable = s;
        return self();
    }
    BoxPtr Box::setAutoScroll(bool a)
    {
        autoScroll = a;
        return self();
    }

    int Box::getNumChildren() const { return children.size(); }
    bool Box::getPadded() const { return padded; }
    bool Box::getScrollable() const { return scrollable; }
    bool Box::getAutoScroll() const { return autoScroll; }

    VBox::VBox() : Box() {}
    void VBox::renderControl()
    {
        if (!visible)
            return;
        ImGui::PushID(this);

        ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame;
        if(scrollable) {
            table_flags |= ImGuiTableFlags_ScrollY;
        }

        if (ImGui::BeginTable("##vbox_layout", 1, table_flags))
        {
            for (auto &child : children)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (child.stretchy)
                {
                    ImGui::PushItemWidth(-1);
                }
                child.control->render();
                if (child.stretchy)
                {
                    ImGui::PopItemWidth();
                }
                if(padded) {
                    ImGui::Spacing();
                }
            }
            ImGui::EndTable();
        }

        if (scrollable)
        {
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }
        }

        ImGui::PopID();
    }

    HBox::HBox() : Box() {}
    void HBox::renderControl()
    {
        if (!visible)
            return;
        ImGui::PushID(this);

        ImGuiTableFlags table_flags = 0;
        if(scrollable) {
            table_flags |= ImGuiTableFlags_ScrollX;
        }

        if (ImGui::BeginTable("##hbox_layout", (int)children.size(), table_flags))
        {
            for (size_t i = 0; i < children.size(); ++i)
            {
                ImGuiTableColumnFlags flags = children[i].stretchy ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_WidthFixed;
                ImGui::TableSetupColumn("", flags);
            }

            ImGui::TableNextRow();

            for (size_t i = 0; i < children.size(); ++i)
            {
                ImGui::TableNextColumn();
                if (children[i].stretchy)
                {
                    ImGui::PushItemWidth(-FLT_MIN);
                }
                children[i].control->render();
                if (children[i].stretchy)
                {
                    ImGui::PopItemWidth();
                }
            }
            ImGui::EndTable();
        }

        ImGui::PopID();
    }
} // namespace mui
