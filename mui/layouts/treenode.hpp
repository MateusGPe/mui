// layouts/treenode.hpp
#pragma once
#include "../widgets/control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <imgui.h>

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
        
        bool defaultOpen = false;
        bool framed = false;
        bool selected = false;

        std::function<void()> onClickCb;
        std::function<void()> onDoubleClickCb;

    public:
        explicit TreeNode(const std::string& label);
        static TreeNodePtr create(const std::string& label) { return std::make_shared<TreeNode>(label); }

        void onHandleDestroyed() override;
        void renderControl() override;

        TreeNodePtr append(IControlPtr child);
        TreeNodePtr setDefaultOpen(bool open);
        TreeNodePtr setFramed(bool f);
        TreeNodePtr setSelected(bool s);
        TreeNodePtr setIconText(const std::string& t);
        TreeNodePtr setIconTexture(ImTextureID tex);
        
        TreeNodePtr onClick(std::function<void()> cb);
        TreeNodePtr onDoubleClick(std::function<void()> cb);
    };
} // namespace mui
