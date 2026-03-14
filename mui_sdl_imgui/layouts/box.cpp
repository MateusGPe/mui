#include "box.hpp"
#include "app.hpp"
#include <imgui.h>
#include <cfloat> // For FLT_MIN

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
    void VBox::render()
    {
        if (!visible)
            return;
        ImGui::PushID(this);

        if (scrollable)
        {
            // ImVec2(0,0) uses remaining available space.
            ImGui::BeginChild("##vbox_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        }

        for (auto &child : children)
        {
            if (child.stretchy)
            {
                ImGui::PushItemWidth(-1.0f);
            }
            child.control->render();
            if (child.stretchy)
            {
                ImGui::PopItemWidth();
            }
            if (padded)
                ImGui::Spacing();
        }

        if (scrollable)
        {
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
        }

        ImGui::PopID();
    }

    HBox::HBox() : Box() {}
    void HBox::render()
    {
        if (!visible)
            return;
        ImGui::PushID(this);

        if (scrollable)
        {
            ImGui::BeginChild("##hbox_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        }

        // Using a table is the most robust way to handle horizontal layouts with stretching.
        if (ImGui::BeginTable("##hbox_layout", (int)children.size()))
        {
            // Define column properties based on the 'stretchy' flag.
            for (size_t i = 0; i < children.size(); ++i)
            {
                ImGuiTableColumnFlags flags = children[i].stretchy ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_WidthFixed;
                ImGui::TableSetupColumn("", flags);
            }

            ImGui::TableNextRow();

            for (size_t i = 0; i < children.size(); ++i)
            {
                ImGui::TableNextColumn();
                // For stretchy items, we need to tell the widget to fill the column.
                if (children[i].stretchy)
                {
                    // Using -FLT_MIN is a common ImGui trick to fill the cell width.
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

        if (scrollable)
        {
            if (autoScroll && ImGui::GetScrollX() >= ImGui::GetScrollMaxX())
            {
                ImGui::SetScrollHereX(1.0f);
            }
            ImGui::EndChild();
        }

        ImGui::PopID();
    }
} // namespace mui
