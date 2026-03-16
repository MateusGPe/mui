// `mui_breadcrumb_bar.hpp
#pragma once
#include <vector>
#include <string>
#include <functional>
#include "control.hpp" // Assuming your mui base class
#include "mui_file_dialog_structs.hpp"

namespace mui {

class BreadcrumbBar : public Control {
public:
    std::vector<BreadcrumbNode> nodes;
    std::string currentPathText;
    bool isTextEntryMode = false;

    // Callbacks
    std::function<void(const std::string& id)> onNodeClicked;
    std::function<void(const std::string& path)> onPathEntered;

    void renderControl() override;
};

} // namespace mui

