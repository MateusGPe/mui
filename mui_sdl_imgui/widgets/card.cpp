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

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 start_cursor_pos = ImGui::GetCursorScreenPos();
        ImVec2 avail_size = ImGui::GetContentRegionAvail();

        // Split the draw list into a background channel (0) and a foreground channel (1)
        draw_list->ChannelsSplit(2);

        // --- FOREGROUND (CHANNEL 1) ---
        // First, render the child content to the foreground to determine its size.
        draw_list->ChannelsSetCurrent(1);
        ImGui::SetCursorScreenPos(ImVec2(start_cursor_pos.x + padding, start_cursor_pos.y + padding));

        // If filling width, constrain the child's rendering environment
        if (fillWidth)
        {
            ImGui::PushItemWidth(avail_size.x - padding * 2);
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

        // Get the bounding box of the child content
        ImVec2 content_p0 = ImGui::GetItemRectMin();
        ImVec2 content_p1 = ImGui::GetItemRectMax();
        ImVec2 content_size = ImVec2(content_p1.x - content_p0.x, content_p1.y - content_p0.y);

        // Determine final card dimensions
        float final_width = fillWidth ? avail_size.x : (content_size.x + padding * 2);
        float final_height = fillHeight ? avail_size.y : (content_size.y + padding * 2);

        // Define card screen coordinates
        ImVec2 card_p0 = start_cursor_pos;
        ImVec2 card_p1 = ImVec2(start_cursor_pos.x + final_width, start_cursor_pos.y + final_height);

        // --- BACKGROUND (CHANNEL 0) ---
        // Now, switch to the background channel to draw behind the content.
        draw_list->ChannelsSetCurrent(0);
        if (shadowAlpha > 0)
        {
            draw_list->AddRectFilled(ImVec2(card_p0.x + shadowOffset, card_p0.y + shadowOffset), ImVec2(card_p1.x + shadowOffset, card_p1.y + shadowOffset), IM_COL32(0, 0, 0, shadowAlpha), 4.0f);
        }
        draw_list->AddRectFilled(card_p0, card_p1, ImGui::GetColorU32(ImGuiCol_ChildBg), 4.0f);
        draw_list->AddRect(card_p0, card_p1, ImGui::GetColorU32(ImGuiCol_Border), 4.0f);

        // Merge the channels back together.
        draw_list->ChannelsMerge();

        // Create an invisible button to occupy the final padded space in the layout.
        ImGui::SetCursorScreenPos(start_cursor_pos);
        ImGui::InvisibleButton("##card_spacer", ImVec2(final_width, final_height));

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

    CardPtr Card::setShadow(float blur, float offset, int alpha)
    {
        verifyState();
        shadowBlur = blur;
        shadowOffset = offset;
        shadowAlpha = alpha;
        return self();
    }
} // namespace mui