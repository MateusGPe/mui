// mui_file_dialog_structs.hpp
#pragma once
#include <string>
#include <vector>

namespace mui {

struct FileInfo {
    std::string id; 
    std::string name;
    bool isDirectory = false;
    size_t size = 0;
    std::string dateModified = "";
    void* icon = nullptr;
    bool hasThumb = false;
};

struct BreadcrumbNode {
    std::string id;
    std::string label;
};

struct FileFilter {
    std::string label;
    std::vector<std::string> extensions;
};

struct TreeNodeData {
    std::string id;
    std::string label;
    void* icon = nullptr;
    bool hasChildren = false;
    bool isExpanded = false;
    bool isSelected = false;
};

} // namespace mui

