// mui_breadcrumb_bar.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "mui_breadcrumb_bar.hpp"
#include <imgui.h>
#include <imgui_internal.h>

#define GUI_ELEMENT_SIZE std::max(ImGui::GetFontSize() + 10.f, 24.f)

namespace mui {

void BreadcrumbBar::renderControl() {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::CalcItemSize(ImVec2(-250, GUI_ELEMENT_SIZE), 200, GUI_ELEMENT_SIZE);
    const ImRect bb(pos, pos + size);

    if (!isTextEntryMode) {
        ImGui::PushClipRect(bb.Min, bb.Max, false);

        bool hovered = ImGui::IsMouseHoveringRect(bb.Min, bb.Max);
        bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        bool anyOtherHC = false;

        // Draw Background
        window->DrawList->AddRectFilled(pos, pos + size, 
            ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]));

        // Calculate button widths
        float totalWidth = 0.0f;
        std::vector<float> btnWidths;
        for (const auto& node : nodes) {
            float elSize = ImGui::CalcTextSize(node.label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + GUI_ELEMENT_SIZE;
            totalWidth += elSize;
            btnWidths.push_back(elSize);
        }
        totalWidth -= GUI_ELEMENT_SIZE; // Remove padding for the last element

        // Render Buttons
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        
        bool isFirstElement = true;
        for (size_t i = 0; i < nodes.size(); i++) {
            if (totalWidth > size.x - 30 && i != nodes.size() - 1) {
                totalWidth -= btnWidths[i];
                continue; // Trim to fit
            }

            ImGui::PushID(static_cast<int>(i));
            if (!isFirstElement) {
                ImGui::ArrowButtonEx("##dir_dropdown", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));
                anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
                ImGui::SameLine();
            }
            
            if (ImGui::Button(nodes[i].label.c_str(), ImVec2(0, GUI_ELEMENT_SIZE))) {
                if (onNodeClicked) onNodeClicked(nodes[i].id);
            }
            anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
            ImGui::SameLine();
            ImGui::PopID();

            isFirstElement = false;
        }
        ImGui::PopStyleVar(2);

        // State transitions
        if (!anyOtherHC && clicked) {
            isTextEntryMode = true;
            // Setup text for entry mode
            currentPathText = "";
            for (const auto& n : nodes) currentPathText += n.id; // Assemble full path
        }

        ImGui::PopClipRect();
        ImGui::SetCursorScreenPos(pos + ImVec2(0, size.y + ImGui::GetStyle().ItemSpacing.y));
    } 
    else {
        // Text Input Mode
        ImGui::SetKeyboardFocusHere();
        
        // Use a static buffer or ImGui::InputText callback for std::string
        char buffer[1024];
        strncpy(buffer, currentPathText.c_str(), sizeof(buffer));
        
        if (ImGui::InputTextEx("##pathbox_input", "", buffer, 1024, ImVec2(-250, GUI_ELEMENT_SIZE), ImGuiInputTextFlags_EnterReturnsTrue)) {
            currentPathText = buffer;
            isTextEntryMode = false;
            if (onPathEntered) onPathEntered(currentPathText);
        }

        if (!ImGui::IsItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            isTextEntryMode = false; // Revert if lost focus
        }
    }
}

} // namespace mui
