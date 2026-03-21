// layouts/treenode.cpp
#include "treenode.hpp"
#include "../core/app.hpp"
#include <imgui.h>

namespace mui
{
    TreeNode::TreeNode(const std::string& label) : label(label) { App::assertMainThread(); }

    void TreeNode::onHandleDestroyed()
    {
        Control<TreeNode>::onHandleDestroyed();
        for (auto& child : children) {
            child->onHandleDestroyed();
        }
    }

    void TreeNode::renderControl()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (defaultOpen) flags |= ImGuiTreeNodeFlags_DefaultOpen;
        if (framed) flags |= ImGuiTreeNodeFlags_Framed;
        if (spanAvailWidth) flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        if (selected) flags |= ImGuiTreeNodeFlags_Selected;
        if (children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;

        // Smart string formatting to make room for icons
        std::string displayStr = label;
        if (!iconText.empty()) {
            displayStr = iconText + " " + label;
        } else if (iconTex) {
            displayStr = "    " + label; // Leave padding for the image texture
        }

        bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags, "%s", displayStr.c_str());

        // Event hooks
        if (ImGui::IsItemClicked(0) && onClickCb) onClickCb();
        if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered() && onDoubleClickCb) onDoubleClickCb();

        // Draw image manually right into the space we left for it
        if (iconTex) {
            ImVec2 min = ImGui::GetItemRectMin();
            float fontSize = ImGui::GetFontSize();
            
            // Calculate exact position after the drop-down arrow
            float iconX = min.x + ImGui::GetTreeNodeToLabelSpacing() - fontSize - ImGui::GetStyle().ItemInnerSpacing.x;
            float iconY = min.y + (ImGui::GetItemRectSize().y - fontSize) * 0.5f;
            
            ImGui::GetWindowDrawList()->AddImage(iconTex, ImVec2(iconX, iconY), ImVec2(iconX + fontSize, iconY + fontSize));
        }

        if (nodeOpen) {
            for (auto& child : children) {
                child->render();
            }
            ImGui::TreePop();
        }

        renderTooltip();
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    TreeNodePtr TreeNode::append(IControlPtr child) { verifyState(); children.push_back(child); return self(); }
    TreeNodePtr TreeNode::setDefaultOpen(bool open) { defaultOpen = open; return self(); }
    TreeNodePtr TreeNode::setFramed(bool f) { framed = f; return self(); }
    TreeNodePtr TreeNode::setSelected(bool s) { selected = s; return self(); }
    TreeNodePtr TreeNode::setIconText(const std::string& t) { iconText = t; return self(); }
    TreeNodePtr TreeNode::setIconTexture(ImTextureID tex) { iconTex = tex; return self(); }
    TreeNodePtr TreeNode::onClick(std::function<void()> cb) { onClickCb = std::move(cb); return self(); }
    TreeNodePtr TreeNode::onDoubleClick(std::function<void()> cb) { onDoubleClickCb = std::move(cb); return self(); }
} // namespace mui
