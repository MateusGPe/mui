// mui_tree_view.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "mui_tree_view.hpp"
#include <imgui.h>
#include <imgui_internal.h>

// Original FontAwesome definitions from ImFileDialog
#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "\xef\x81\xbb"
#endif

namespace mui
{

    static void renderNode(TreeNodeData &node,
                           std::function<void(const std::string &)> &onSelect,
                           std::function<void(const std::string &)> &onExpand)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        ImGuiContext &g = *GImGui;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (node.isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;
        if (!node.hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf;

        // We use a unique ID based on the file path (node.id)
        bool is_open = ImGui::TreeNodeEx(node.id.c_str(), flags, "%s", node.label.c_str());

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            if (onSelect)
                onSelect(node.id);
        }

        if (is_open)
        {
            if (node.hasChildren && onExpand)
            {
                onExpand(node.id);
            }
            // In a lazy-load scenario, children would be populated here by the controller
            // for (auto& child : node.children) renderNode(child);
            ImGui::TreePop();
        }
    }
    void TreeView::renderControl()
    {
        for (auto &rootNode : roots)
        {
            renderNode(rootNode, onNodeSelected, onNodeExpanded);
        }
    }
} // namespace mui
