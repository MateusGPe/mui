// layouts/splitter.cpp
#include "splitter.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"
#include <algorithm>

namespace mui
{
    SplitterView::SplitterView(SplitterOrientation o) : orientation(o) { App::assertMainThread(); }

    void SplitterView::renderControl()
    {
        if (!visible) return;
        ScopedID id(this);

        ImVec2 avail = ImGui::GetContentRegionAvail();
        
        if (orientation == SplitterOrientation::Horizontal)
        {
            float size1 = (avail.x - thickness) * splitRatio;
            float size2 = avail.x - thickness - size1;

            // Panel 1
            ImGui::BeginChild("##panel1", ImVec2(size1, avail.y), false, ImGuiWindowFlags_NoSavedSettings);
            if (panel1) panel1->render();
            ImGui::EndChild();

            ImGui::SameLine(0, 0);

            // Splitter
            ImGui::Button("##splitter", ImVec2(thickness, avail.y));
            if (ImGui::IsItemActive())
            {
                float delta = ImGui::GetIO().MouseDelta.x;
                splitRatio += delta / avail.x;
                splitRatio = std::clamp(splitRatio, 0.05f, 0.95f);
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            ImGui::SameLine(0, 0);

            // Panel 2
            ImGui::BeginChild("##panel2", ImVec2(size2, avail.y), false, ImGuiWindowFlags_NoSavedSettings);
            if (panel2) panel2->render();
            ImGui::EndChild();
        }
        else
        {
            float size1 = (avail.y - thickness) * splitRatio;
            float size2 = avail.y - thickness - size1;

            // Panel 1
            ImGui::BeginChild("##panel1", ImVec2(avail.x, size1), false, ImGuiWindowFlags_NoSavedSettings);
            if (panel1) panel1->render();
            ImGui::EndChild();

            // Splitter
            ImGui::Button("##splitter", ImVec2(avail.x, thickness));
            if (ImGui::IsItemActive())
            {
                float delta = ImGui::GetIO().MouseDelta.y;
                splitRatio += delta / avail.y;
                splitRatio = std::clamp(splitRatio, 0.05f, 0.95f);
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

            // Panel 2
            ImGui::BeginChild("##panel2", ImVec2(avail.x, size2), false, ImGuiWindowFlags_NoSavedSettings);
            if (panel2) panel2->render();
            ImGui::EndChild();
        }
    }

    SplitterViewPtr SplitterView::setPanel1(IControlPtr c) { panel1 = c; return self(); }
    SplitterViewPtr SplitterView::setPanel2(IControlPtr c) { panel2 = c; return self(); }
    SplitterViewPtr SplitterView::setSplitRatio(float ratio) { splitRatio = std::clamp(ratio, 0.05f, 0.95f); return self(); }
    SplitterViewPtr SplitterView::setThickness(float t) { thickness = t; return self(); }
} // namespace mui
