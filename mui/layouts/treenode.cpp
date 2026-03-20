// layouts/treenode.cpp
#include "treenode.hpp"
#include "../core/app.hpp"
#include <imgui.h>

namespace mui
{
    TreeNode::TreeNode(const std::string& label) : label(label) { App::assertMainThread(); }

    void TreeNode::onHandleDestroyed()
    {
        Control::onHandleDestroyed();
        for (auto& child : children) {
            child->onHandleDestroyed();
        }
    }

    void TreeNode::renderControl()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiTreeNodeFlags flags = 0;
        if (defaultOpen) flags |= ImGuiTreeNodeFlags_DefaultOpen;
        if (framed) flags |= ImGuiTreeNodeFlags_Framed;
        if (spanAvailWidth) flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);
        
        renderTooltip();

        if (nodeOpen) {
            for (auto& child : children) {
                child->render();
            }
            ImGui::TreePop();
        }

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    TreeNodePtr TreeNode::append(IControlPtr child) {
        verifyState();
        children.push_back(child);
        return self();
    }

    TreeNodePtr TreeNode::setDefaultOpen(bool open) { defaultOpen = open; return self(); }
    TreeNodePtr TreeNode::setFramed(bool f) { framed = f; return self(); }
    TreeNodePtr TreeNode::setSpanAvailWidth(bool span) { spanAvailWidth = span; return self(); }
} // namespace mui