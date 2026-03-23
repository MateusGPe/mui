// layouts/treenode.hpp
#pragma once
#include "../widgets/control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <imgui.h> // For ImTextureID
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class TreeNode;
    using TreeNodePtr = std::shared_ptr<TreeNode>;

    class TreeNode : public Control<TreeNode>
    {
    protected:
        std::string label;
        std::string iconText;
        ImTextureID iconTex = 0;
        std::vector<IControlPtr> children;
        ImGuiTreeNodeFlags m_flags;
    
        explicit TreeNode(const std::string& label);

    public:
        mui::Signal<> onClickSignal;
        mui::Signal<> onDoubleClickSignal;
    
        static TreeNodePtr create(const std::string& label) { return std::shared_ptr<TreeNode>(new TreeNode(label)); }
    
        void onHandleDestroyed() override;
        void renderControl() override;
    
        TreeNodePtr append(IControlPtr child);
        TreeNodePtr setDefaultOpen(bool open);
        TreeNodePtr setFramed(bool f);
        TreeNodePtr setSelected(bool s);
        TreeNodePtr setIconText(const std::string& t);
        TreeNodePtr setIconTexture(ImTextureID tex);
        TreeNodePtr setAllowOverlap(bool b);
        TreeNodePtr setBullet(bool b);
        TreeNodePtr setSpanFullWidth(bool b);
        
        TreeNodePtr onClick(std::function<void()> cb);
        TreeNodePtr onDoubleClick(std::function<void()> cb);
    
        bool isSelected() const;
        TreeNodePtr bindSelected(std::shared_ptr<Observable<bool>> observable);

        // ImGuiTreeNodeFlags additions
        TreeNodePtr setOpenOnDoubleClick(bool b);
        TreeNodePtr setOpenOnArrow(bool b);
        TreeNodePtr setLeaf(bool b);
        TreeNodePtr setNoTreePushOnOpen(bool b);
        TreeNodePtr setNoAutoOpenOnLog(bool b);
        TreeNodePtr setCollapsingHeader(bool b);
        TreeNodePtr setSpanAvailWidth(bool b);
        TreeNodePtr setSpanAllColumns(bool b);
        TreeNodePtr setNavLeftJumpsToParent(bool b);
        TreeNodePtr setFramePadding(bool b);
    };
} // namespace mui