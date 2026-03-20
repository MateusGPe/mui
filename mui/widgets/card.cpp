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

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().ChildRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

        // Explicitly set the next window size.
        // Passing 0.0f forces an auto-fit on that axis.
        // Passing GetContentRegionAvail() forces it to fill available space.
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImGui::SetNextWindowSize(ImVec2(
            spanAvailWidth ? avail.x : 0.0f,
            fillHeight ? avail.y : 0.0f));

        ImGuiChildFlags child_flags = ImGuiChildFlags_Borders;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;

        // Apply auto-resize flags for the axes designated as 0.0f
        if (!spanAvailWidth)
        {
            child_flags |= ImGuiChildFlags_AutoResizeX;
        }
        if (!fillHeight)
        {
            child_flags |= ImGuiChildFlags_AutoResizeY;
        }

        // Size parameter is ImVec2(0,0) because SetNextWindowSize dictates the dimensions
        ImGui::BeginChild("##card_content_host", ImVec2(0.0f, 0.0f), child_flags, window_flags);

        if (child)
            child->render();

        ImGui::EndChild();

        ImGui::PopStyleVar(3);
        ImGui::PopID();
    }

    CardPtr Card::setChild(IControlPtr c)
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

    CardPtr Card::setFillHeight(bool fill)
    {
        verifyState();
        fillHeight = fill;
        return self();
    }
} // namespace mui