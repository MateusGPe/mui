#include "card.hpp"
#include "app.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace mui
{
    Card::Card()
    {
        App::assertMainThread();
    }

    void Card::renderControl()
    {
        if (!visible)
            return;

        ImGui::PushID(this);

        // We can't know the size of the child ahead of time.
        // So, we first render the child content off-screen to determine its size.
        // This is a common pattern in ImGui for complex layouts.
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
        ImGui::BeginGroup();
        if (child)
        {
            child->render();
        }
        ImGui::EndGroup();
        ImGui::PopStyleVar();

        ImVec2 content_size = ImGui::GetItemRectSize();
        ImVec2 avail_size = ImGui::GetContentRegionAvail();

        // Determine final card dimensions
        float final_width = fillWidth ? avail_size.x : (content_size.x + padding * 2);
        float final_height = fillHeight ? avail_size.y : (content_size.y + padding * 2);
        ImVec2 final_size(final_width, final_height);

        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
        const ImRect bb(cursor_pos, ImVec2(cursor_pos.x + final_size.x, cursor_pos.y + final_size.y));

        ImGui::ItemSize(bb.GetSize());
        if(ImGui::ItemAdd(bb, 0))
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_ChildBg), 4.0f);
            draw_list->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Border), 4.0f);

            // Now render the child for real
            ImGui::SetCursorScreenPos(ImVec2(bb.Min.x + padding, bb.Min.y + padding));
            if (fillWidth)
            {
                ImGui::PushItemWidth(final_width - padding * 2);
            }
            ImGui::BeginGroup();
            if (child)
            {
                child->render();
            }
            ImGui::EndGroup();
            if (fillWidth)
            {
                ImGui::PopItemWidth();
            }
        }


        ImGui::PopID();
    }

    CardPtr Card::setChild(ControlPtr c)
    {
        verifyState();
        child = c;
        return self();
    }

    CardPtr Card::setPadding(float p)
    {
        verifyState();
        padding = p;
        return self();
    }

    CardPtr Card::setFillWidth(bool fill)
    {
        verifyState();
        fillWidth = fill;
        return self();
    }

    CardPtr Card::setFillHeight(bool fill)
    {
        verifyState();
        fillHeight = fill;
        return self();
    }
} // namespace mui