namespace mui {

// Used by IconGrid and TreeView
struct FileInfo {
    std::string id;         // The absolute path (used as unique identifier)
    std::string name;       // Display name (e.g., "document.txt" or "C:")
    bool isDirectory;
    size_t size;            // File size in bytes (for detail view)
    std::string dateModified;
    void* icon;             // ImTextureID (null if pending/default)
    bool hasThumb;          // True if a custom thumbnail was loaded
};

// Used by BreadcrumbBar
struct BreadcrumbNode {
    std::string id;         // Absolute path segment
    std::string label;      // Display name
};

// Used by BottomBar extension dropdown
struct FileFilter {
    std::string label;                  // e.g., "Image Files (*.png *.jpg)"
    std::vector<std::string> extensions; // e.g., {".png", ".jpg"}
};

}

