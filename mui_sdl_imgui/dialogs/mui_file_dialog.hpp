// mui_file_dialog.hpp
#pragma once
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <string>

// Include the generic data structures and widgets
#include "mui_file_dialog_structs.hpp"
#include "mui_breadcrumb_bar.hpp"
#include "mui_tree_view.hpp"
#include "mui_icon_grid.hpp"
#include "mui_file_dialog_layouts.hpp"

namespace mui {

enum class FileDialogType {
    OpenFile,
    OpenFiles,
    OpenDirectory,
    SaveFile
};

struct FileDialogState {
    std::string key;
    std::string title;
    FileDialogType type;
    std::string startingDir;
    std::function<void(const std::vector<std::string>&)> callback;
    bool open = false;
};

class FileDialog {
public:
    static FileDialog& Instance() {
        static FileDialog ret;
        return ret;
    }

    FileDialog();
    ~FileDialog();

    void Open(FileDialogType type, const std::string& key, const std::string& title, const std::function<void(const std::vector<std::string>&)>& callback, const std::string& startingDir = "");
    void render(); // To be called in your main UI loop

private:
    std::unordered_map<std::string, FileDialogState> dialogs;
    // --- UI Composites ---
    FileDialogTopBar topBar;
    TreeView sidebar;
    IconGrid mainGrid;
    FileDialogBottomBar bottomBar;

    // --- OS State ---
    std::filesystem::path currentDirectory;
    std::vector<std::filesystem::path> backHistory;
    std::vector<std::filesystem::path> forwardHistory;
    
    // --- Background Threading & Icons ---
    std::thread* previewLoader;
    std::atomic<bool> previewLoaderRunning;
    std::mutex imageQueueMutex;
    
    struct PendingPreview {
        std::string filepath;
        int width, height, channels;
        unsigned char* data;
    };
    std::vector<PendingPreview> pendingPreviews;
    std::unordered_map<std::string, void*> loadedTextures;

    // --- Core Logic Methods ---
    void bindCallbacks();
    void loadDirectory(const std::filesystem::path& path);
    void populateDrives();
    
    // Original threading logic preserved
    void stopPreviewLoader();
    void startPreviewLoader();
    void processLoadedPreviewsOnMainThread();
};

} // namespace mui

