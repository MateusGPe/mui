// mui_icon_grid.hpp
#pragma once
#include <vector>
#include <string>
#include <functional>
#include "control.hpp"
#include "mui_file_dialog_structs.hpp"

namespace mui {

class IconGrid : public Control {
public:
    std::vector<FileInfo> items;
    float zoomLevel = 1.0f; // Scale base size (32px) by zoom
    std::vector<std::string> selectedIds;

    // Callbacks
    std::function<void(const std::string& id)> onItemClicked;
    std::function<void(const std::string& id)> onItemDoubleClicked;
    std::function<void(float newZoom)> onZoomChanged;

    void renderControl() override;

private:
    void renderTableView();
    void renderGridView();
};

} // namespace mui
