// mui_tree_view.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "mui_tree_view.hpp"
#include <imgui.h>
#include <imgui_internal.h>

// Original FontAwesome definitions from ImFileDialog
#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "\xef\x81\xbb"
#endif

namespace mui {

static void RenderNodeRecursive(TreeNodeData& node, 
                                std::function<void(const std::string&)>& onSelect, 
                                std::function<void(const std::string&)>& onExpand) 
{
    ImGuiContext& g = *GImGui;
    (void)g;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImU32 id = window->GetID(node.id.c_str());
    int opened = window->StateStorage.GetInt(id, 0);
    if (node.isExpanded) opened = 1;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool is_mouse_x_over_arrow = (ImGui::GetIO().MousePos.x >= pos.x && ImGui::GetIO().MousePos.x < pos.x + ImGui::GetFontSize());
    
    // Invisible hit box for the node line
    if (ImGui::InvisibleButton(node.id.c_str(), ImVec2(-FLT_MIN, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2))) {
        if (is_mouse_x_over_arrow && node.hasChildren) {
            opened = !opened;
            window->StateStorage.SetInt(id, opened);
            if (opened && onExpand) onExpand(node.id);
        } else {
            if (onSelect) onSelect(node.id);
        }
    }

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
    
    if (doubleClick && hovered && node.hasChildren) {
        opened = !opened;
        window->StateStorage.SetInt(id, opened);
        if (opened && onExpand) onExpand(node.id);
    }

    if (hovered || active || node.isSelected) {
        window->DrawList->AddRectFilled(
            ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 
            ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : (node.isSelected ? ImGuiCol_Header : ImGuiCol_HeaderHovered)])
        );
    }

    float iconSize = ImGui::GetFontSize() + 3.0f;
    float icon_posX = pos.x + ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y;
    float text_posX = icon_posX + ImGui::GetStyle().FramePadding.y + iconSize;

    // Draw Arrow
    if (node.hasChildren) {
        ImGui::RenderArrow(window->DrawList, ImVec2(pos.x, pos.y + ImGui::GetStyle().FramePadding.y), 
                           ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[((hovered && is_mouse_x_over_arrow) || opened) ? ImGuiCol_Text : ImGuiCol_TextDisabled]), 
                           opened ? ImGuiDir_Down : ImGuiDir_Right);
    }

    // Draw Icon
    if (node.icon) {
        window->DrawList->AddImage((ImTextureID)node.icon, ImVec2(icon_posX, pos.y), ImVec2(icon_posX + iconSize, pos.y + iconSize));
    } else {
        window->DrawList->AddText(ImGui::GetFont(), iconSize, ImVec2(icon_posX, pos.y + ImGui::GetStyle().FramePadding.y), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), ICON_FA_FOLDER);
    }

    // Draw Text
    ImGui::RenderText(ImVec2(text_posX, pos.y + ImGui::GetStyle().FramePadding.y), node.label.c_str());

    // Recursion handles child nodes if expanded. 
    // In a pure MVC setup, the generic tree view would have its children populated directly in the TreeNodeData struct by the Controller upon `onExpand`.
    if (opened) {
        ImGui::TreePush(node.id.c_str());
        // For 100% parity, your generic TreeNodeData struct will need a std::vector<TreeNodeData> children array added to it 
        // to recurse through here.
        ImGui::TreePop();
    }
}

void TreeView::renderControl() {
    for (auto& rootNode : roots) {
        RenderNodeRecursive(rootNode, onNodeSelected, onNodeExpanded);
    }
}

} // namespace mui

