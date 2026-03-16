// mui_file_dialog_layouts.hpp
#pragma once
#include <string>
#include <vector>
#include <functional>
#include "control.hpp"
#include "mui_file_dialog_structs.hpp"

namespace mui {

class FileDialogTopBar : public Control {
public:
    std::string searchInput;

    // Callbacks
    std::function<void()> onBackClicked;
    std::function<void()> onForwardClicked;
    std::function<void()> onUpClicked;
    std::function<void(const std::string& query)> onSearchChanged;

    void renderControl() override;
};

class FileDialogBottomBar : public Control {
public:
    std::string filenameInput;
    std::vector<FileFilter> filters;
    int selectedFilterIndex = 0;
    bool isDirectoryMode = false;

    // Callbacks
    std::function<void(const std::string& filename)> onFilenameChanged;
    std::function<void(int filterIndex)> onFilterChanged;
    std::function<void()> onCancelClicked;
    std::function<void()> onOkClicked;

    void renderControl() override;
};

} // namespace mui

