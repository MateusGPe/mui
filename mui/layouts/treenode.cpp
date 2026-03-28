// layouts/treenode.cpp
#include "treenode.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    TreeNode::TreeNode(const std::string& label) 
        : label(label)
    { 
        App::assertMainThread();
        m_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    }

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
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiTreeNodeFlags flags = m_flags;
        if (spanAvailWidth) flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
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
        if (ImGui::IsItemClicked(0)) onClickSignal();
        if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) onDoubleClickSignal();

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
            for (size_t i = 0; i < children.size(); ++i) {
                auto& child = children[i];
                bool needsIndexId = child->getID().empty();
                if (needsIndexId) ImGui::PushID(static_cast<int>(i));
                child->render();
                if (needsIndexId) ImGui::PopID();
            }
            ImGui::TreePop();
        }

        renderTooltip();
        ImGui::EndDisabled();
    }

    TreeNodePtr TreeNode::append(IControlPtr child) { verifyState(); children.push_back(child); return self(); }
    TreeNodePtr TreeNode::setDefaultOpen(bool open) {
        if (open) m_flags |= ImGuiTreeNodeFlags_DefaultOpen;
        else m_flags &= ~ImGuiTreeNodeFlags_DefaultOpen;
        return self();
    }
    TreeNodePtr TreeNode::setFramed(bool f) {
        if (f) m_flags |= ImGuiTreeNodeFlags_Framed;
        else m_flags &= ~ImGuiTreeNodeFlags_Framed;
        return self();
    }
    TreeNodePtr TreeNode::setSelected(bool s) {
        if (s) m_flags |= ImGuiTreeNodeFlags_Selected;
        else m_flags &= ~ImGuiTreeNodeFlags_Selected;
        return self();
    }
    TreeNodePtr TreeNode::setIconText(const std::string& t) { iconText = t; return self(); }
    TreeNodePtr TreeNode::setIconTexture(ImTextureID tex) { iconTex = tex; return self(); }
    TreeNodePtr TreeNode::setAllowOverlap(bool b) {
        if (b) m_flags |= ImGuiTreeNodeFlags_AllowOverlap;
        else m_flags &= ~ImGuiTreeNodeFlags_AllowOverlap;
        return self();
    }
    TreeNodePtr TreeNode::setBullet(bool b) {
        if (b) m_flags |= ImGuiTreeNodeFlags_Bullet;
        else m_flags &= ~ImGuiTreeNodeFlags_Bullet;
        return self();
    }
    TreeNodePtr TreeNode::setSpanFullWidth(bool b) {
        if (b) m_flags |= ImGuiTreeNodeFlags_SpanFullWidth;
        else m_flags &= ~ImGuiTreeNodeFlags_SpanFullWidth;
        return self();
    }
    TreeNodePtr TreeNode::onClick(std::function<void()> cb)
    {
        if (cb) m_connections.push_back(onClickSignal.connect(std::move(cb)));
        return self();
    }
    TreeNodePtr TreeNode::onDoubleClick(std::function<void()> cb)
    {
        if (cb) m_connections.push_back(onDoubleClickSignal.connect(std::move(cb)));
        return self();
    }

    bool TreeNode::isSelected() const { return m_flags & ImGuiTreeNodeFlags_Selected; }
    TreeNodePtr TreeNode::bindSelected(std::shared_ptr<Observable<bool>> observable)
    {
        setSelected(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const bool &val) { mui::App::queueMain([this, val]() { this->setSelected(val); }); }));
        // Two-way binding requires external logic to manage selection state across multiple nodes.
        // The app can use the onClick signal to update the observable.
        return self();
    }
    TreeNodePtr TreeNode::setOpenOnDoubleClick(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick;
        else m_flags &= ~ImGuiTreeNodeFlags_OpenOnDoubleClick;
        return self();
    }
    TreeNodePtr TreeNode::setOpenOnArrow(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
        else m_flags &= ~ImGuiTreeNodeFlags_OpenOnArrow;
        return self();
    }
    TreeNodePtr TreeNode::setLeaf(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_Leaf;
        else m_flags &= ~ImGuiTreeNodeFlags_Leaf;
        return self();
    }
    TreeNodePtr TreeNode::setNoTreePushOnOpen(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
        else m_flags &= ~ImGuiTreeNodeFlags_NoTreePushOnOpen;
        return self();
    }
    TreeNodePtr TreeNode::setNoAutoOpenOnLog(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_NoAutoOpenOnLog;
        else m_flags &= ~ImGuiTreeNodeFlags_NoAutoOpenOnLog;
        return self();
    }
    TreeNodePtr TreeNode::setCollapsingHeader(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_Framed; // CollapsingHeader is essentially a framed TreeNode with no tree push
        else m_flags &= ~ImGuiTreeNodeFlags_Framed;
        return self();
    }
    TreeNodePtr TreeNode::setSpanAvailWidth(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        else m_flags &= ~ImGuiTreeNodeFlags_SpanAvailWidth;
        return self();
    }
    TreeNodePtr TreeNode::setSpanAllColumns(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_SpanAllColumns;
        else m_flags &= ~ImGuiTreeNodeFlags_SpanAllColumns;
        return self();
    }
    TreeNodePtr TreeNode::setNavLeftJumpsToParent(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent;
        else m_flags &= ~ImGuiTreeNodeFlags_NavLeftJumpsToParent;
        return self();
    }
    TreeNodePtr TreeNode::setFramePadding(bool b)
    {
        if (b) m_flags |= ImGuiTreeNodeFlags_FramePadding;
        else m_flags &= ~ImGuiTreeNodeFlags_FramePadding;
        return self();
    }
    
} // namespace mui
