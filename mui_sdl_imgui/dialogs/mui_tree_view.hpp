// `mui_tree_view.hpp`
#pragma once
#include <vector>
#include <string>
#include <functional>
#include "control.hpp"
#include "mui_file_dialog_structs.hpp"

namespace mui {

class TreeView : public Control {
public:
    std::vector<TreeNodeData> roots;

    // Callbacks
    std::function<void(const std::string& id)> onNodeSelected;
    std::function<void(const std::string& id)> onNodeExpanded; // For lazy-loading OS directories

    void renderControl() override;
};

} // namespace mui

