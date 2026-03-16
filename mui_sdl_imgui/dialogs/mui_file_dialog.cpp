// mui_file_dialog.cpp
#include "mui_file_dialog.hpp"
#include <algorithm>
#include <imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Original Platform Logic Preserved exactly
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace mui {

FileDialog::FileDialog() {
    previewLoader = nullptr;
    previewLoaderRunning = false;
    bindCallbacks();
    populateDrives();
}

FileDialog::~FileDialog() {
    stopPreviewLoader();
}

void FileDialog::bindCallbacks() {
    // 1. Sidebar to Controller: User clicked a drive or folder in the sidebar
    sidebar.onNodeSelected = [this](const std::string& id) {
        std::filesystem::path p(id);
        if (std::filesystem::exists(p)) {
            loadDirectory(p);
        }
    };

    // 2. Grid to Controller: User double-clicked a folder/file in the main view
    mainGrid.onItemDoubleClicked = [this](const std::string& id) {
        std::filesystem::path p(id);
        if (std::filesystem::is_directory(p)) {
            loadDirectory(p);
        } else {
            // Finalize file selection
            // m_finalize(p.string());
        }
    };

    // 3. TopBar to Controller: User navigated via breadcrumbs or arrows
    topBar.onBackClicked = [this]() {
        if (!backHistory.empty()) {
            forwardHistory.push_back(currentDirectory);
            loadDirectory(backHistory.back());
            backHistory.pop_back();
        }
    };
}

// OS Logic Isolated here
void FileDialog::populateDrives() {
    std::vector<TreeNodeData> driveRoots;

#ifdef _WIN32
    DWORD d = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (d & (1 << i)) {
            std::string driveName = std::string(1, 'A' + i) + ":\\";
            TreeNodeData node;
            node.id = driveName;
            node.label = driveName;
            node.hasChildren = true;
            driveRoots.push_back(node);
        }
    }
#else
    // POSIX fallback
    TreeNodeData rootNode;
    rootNode.id = "/";
    rootNode.label = "Computer";
    rootNode.hasChildren = true;
    driveRoots.push_back(rootNode);
#endif

    sidebar.roots = driveRoots; // Feed generic UI
}

void FileDialog::loadDirectory(const std::filesystem::path& path) {
    currentDirectory = path;
    mainGrid.items.clear();
    stopPreviewLoader();

    // Standard std::filesystem iteration
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        FileInfo info;
        info.id = entry.path().string();
        info.name = entry.path().filename().string();
        info.isDirectory = entry.is_directory();
        
        if (!info.isDirectory) {
            info.size = entry.file_size();
        }
        
        mainGrid.items.push_back(info);
    }

    startPreviewLoader();
}

// Background Threading logic preserved exactly as requested
void FileDialog::startPreviewLoader() {
    previewLoaderRunning = true;
    previewLoader = new std::thread([this]() {
        for (const auto& item : mainGrid.items) {
            if (!previewLoaderRunning) break;
            if (item.isDirectory) continue;

            // Simplified stb_image check (add your specific extensions)
            int width, height, channels;
            unsigned char* data = stbi_load(item.id.c_str(), &width, &height, &channels, 4);
            
            if (data) {
                std::lock_guard<std::mutex> lock(imageQueueMutex);
                pendingPreviews.push_back({item.id, width, height, channels, data});
            }
        }
        previewLoaderRunning = false;
    });
}

void FileDialog::stopPreviewLoader() {
    if (previewLoaderRunning) {
        previewLoaderRunning = false;
        if (previewLoader && previewLoader->joinable()) {
            previewLoader->join();
        }
        delete previewLoader;
        previewLoader = nullptr;
    }
}

void FileDialog::processLoadedPreviewsOnMainThread() {
    std::lock_guard<std::mutex> lock(imageQueueMutex);
    for (auto& preview : pendingPreviews) {
        // Here you generate the ImGui/OpenGL/SDL Texture exactly as your engine handles it
        // void* textureID = GenerateTextureFromData(preview.data, preview.width, preview.height);
        void* textureID = nullptr; // Placeholder
        
        loadedTextures[preview.filepath] = textureID;
        stbi_image_free(preview.data);

        // Update the generic UI model
        for (auto& item : mainGrid.items) {
            if (item.id == preview.filepath) {
                item.icon = textureID;
                item.hasThumb = true;
                break;
            }
        }
    }
    pendingPreviews.clear();
}

void FileDialog::Open(FileDialogType type, const std::string& key, const std::string& title, const std::function<void(const std::vector<std::string>&)>& callback, const std::string& startingDir)
{
    if (dialogs.find(key) == dialogs.end()) {
        dialogs[key] = FileDialogState();
    }

    FileDialogState& state = dialogs[key];
    state.key = key;
    state.title = title;
    state.type = type;
    state.startingDir = startingDir;
    state.callback = callback;
    state.open = true;

    if (!startingDir.empty() && std::filesystem::exists(startingDir)) {
        loadDirectory(startingDir);
    } else {
        populateDrives();
    }
}

void FileDialog::render()
{
    // 1. Process thread-safe queues on main thread
    processLoadedPreviewsOnMainThread();

    for (auto& [key, dialog] : dialogs) {
        if (dialog.open) {
            ImGui::Begin(dialog.title.c_str(), &dialog.open);

            // 2. Render the layout wrappers
            topBar.renderControl();
            
            // Use an ImGui table or your mui::SplitPane to separate sidebar and mainGrid
            sidebar.renderControl();
            ImGui::SameLine();
            mainGrid.renderControl();
            
            bottomBar.renderControl();

            ImGui::End();
        }
    }
}

} // namespace mui

