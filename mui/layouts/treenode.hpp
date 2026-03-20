// layouts/treenode.hpp
#pragma once
#include "../widgets/control.hpp"
#include <string>
#include <vector>
#include <memory>

namespace mui
{
    class TreeNode;
    using TreeNodePtr = std::shared_ptr<TreeNode>;

    class TreeNode : public Control<TreeNode>
    {
    protected:
        std::string label;
        std::vector<IControlPtr> children;
        bool defaultOpen = false;
        bool framed = false;
        bool spanAvailWidth = false;

    public:
        explicit TreeNode(const std::string& label);
        static TreeNodePtr create(const std::string& label) { return std::make_shared<TreeNode>(label); }

        void onHandleDestroyed() override;
        void renderControl() override;

        TreeNodePtr append(IControlPtr child);
        TreeNodePtr setDefaultOpen(bool open);
        TreeNodePtr setFramed(bool f);
        TreeNodePtr setSpanAvailWidth(bool span);
    };
} // namespace mui