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

    class TreeNode : public Control
    {
    protected:
        std::string label;
        std::vector<ControlPtr> children;
        bool defaultOpen = false;
        bool framed = false;

        TreeNodePtr self() { return std::static_pointer_cast<TreeNode>(shared_from_this()); }

    public:
        explicit TreeNode(const std::string& label);
        static TreeNodePtr create(const std::string& label) { return std::make_shared<TreeNode>(label); }

        void onHandleDestroyed() override;
        void render() override;

        TreeNodePtr append(ControlPtr child);
        TreeNodePtr setDefaultOpen(bool open);
        TreeNodePtr setFramed(bool f);
    };
} // namespace mui