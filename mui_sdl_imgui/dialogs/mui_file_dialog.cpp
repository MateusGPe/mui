// mui_file_dialog.cpp
#include "mui_file_dialog.hpp"
#include <algorithm>
#include <imgui.h>
#include "stb_image.h"
#include "../core/app.hpp"

// Original Platform Logic Preserved exactly
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <SDL3/SDL.h>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

namespace mui
{

    FileDialog::FileDialog()
    {
        previewLoader = nullptr;
        previewLoaderRunning = false;
        populateDrives();
    }

   FileDialog::~FileDialog() {
    stopPreviewLoader();
    for (auto const& [path, tex] : loadedTextures) {
        if (g_use_opengl) {
            GLuint texID = (GLuint)(intptr_t)tex;
            glDeleteTextures(1, &texID);
        } else if (tex) {
            SDL_DestroyTexture((SDL_Texture*)tex);
        }
    }
    loadedTextures.clear();
}

    void FileDialog::bindCallbacks(FileDialogState &dialog)
    {
        sidebar.onNodeSelected = [this](const std::string &id)
        {
            std::filesystem::path p(id);
            if (std::filesystem::exists(p))
            {
                loadDirectory(p);
            }
        };

        mainGrid.onItemDoubleClicked = [this, &dialog](const std::string &id)
        {
            std::filesystem::path p(id);
            if (std::filesystem::is_directory(p))
            {
                loadDirectory(p);
            }
            else if (dialog.type == FileDialogType::OpenFile)
            {
                dialog.callback({p.string()});
                dialog.open = false;
            }
        };

        topBar.onBackClicked = [this]()
        {
            if (!backHistory.empty())
            {
                forwardHistory.push_back(currentDirectory);
                loadDirectory(backHistory.back());
                backHistory.pop_back();
            }
        };

        topBar.onForwardClicked = [this]()
        {
            if (!forwardHistory.empty())
            {
                backHistory.push_back(currentDirectory);
                loadDirectory(forwardHistory.back());
                forwardHistory.pop_back();
            }
        };

        bottomBar.onOkClicked = [this, &dialog]()
        {
            if (dialog.type == FileDialogType::OpenFiles)
            {
                dialog.callback(mainGrid.selectedIds);
            }
            else if (dialog.type == FileDialogType::OpenDirectory)
            {
                dialog.callback({currentDirectory.string()});
            }
            else if (dialog.type == FileDialogType::SaveFile)
            {
                std::filesystem::path p = currentDirectory / bottomBar.filenameInput;
                dialog.callback({p.string()});
            }
            dialog.open = false;
        };

        bottomBar.onCancelClicked = [this, &dialog]()
        {
            dialog.open = false;
        };
    }

    // OS Logic Isolated here
    void FileDialog::populateDrives()
    {
        std::vector<TreeNodeData> driveRoots;

#ifdef _WIN32
        DWORD d = GetLogicalDrives();
        for (int i = 0; i < 26; i++)
        {
            if (d & (1 << i))
            {
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

    void FileDialog::loadDirectory(const std::filesystem::path &path)
    {
        currentDirectory = path;
        mainGrid.items.clear();
        stopPreviewLoader();

        // Standard std::filesystem iteration
        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            FileInfo info;
            info.id = entry.path().string();
            info.name = entry.path().filename().string();
            info.isDirectory = entry.is_directory();

            if (!info.isDirectory)
            {
                info.size = entry.file_size();
            }

            mainGrid.items.push_back(info);
        }

        startPreviewLoader();
    }

    // Background Threading logic preserved exactly as requested
    void FileDialog::startPreviewLoader()
    {
        previewLoaderRunning = true;
        previewLoader = new std::thread([this]()
                                        {
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
        previewLoaderRunning = false; });
    }

    void FileDialog::stopPreviewLoader()
    {
        if (previewLoaderRunning)
        {
            previewLoaderRunning = false;
            if (previewLoader && previewLoader->joinable())
            {
                previewLoader->join();
            }
            delete previewLoader;
            previewLoader = nullptr;
        }
    }

    void FileDialog::processLoadedPreviewsOnMainThread()
    {
        std::lock_guard<std::mutex> lock(imageQueueMutex);
        if (pendingPreviews.empty())
            return;

        for (auto &preview : pendingPreviews)
        {
            void *textureID = nullptr;

            if (g_use_opengl) // Accessed via the static flag in app.cpp or mui namespace
            {
                GLuint texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, preview.width, preview.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, preview.data);
                glBindTexture(GL_TEXTURE_2D, 0);
                textureID = (void *)(intptr_t)texture;
            }
            else if (g_renderer)
            {
                SDL_Texture *tex = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, preview.width, preview.height);
                if (tex)
                {
                    SDL_UpdateTexture(tex, nullptr, preview.data, preview.width * 4);
                    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
                    textureID = (void *)tex;
                }
            }

            if (textureID)
            {
                loadedTextures[preview.filepath] = textureID;
                // Update the grid items with the new texture
                for (auto &item : mainGrid.items)
                {
                    if (item.id == preview.filepath)
                    {
                        item.icon = textureID;
                        item.hasThumb = true;
                    }
                }
            }
            stbi_image_free(preview.data);
        }
        pendingPreviews.clear();
    }

    void FileDialog::Open(FileDialogType type, const std::string &key, const std::string &title, const std::function<void(const std::vector<std::string> &)> &callback, const std::string &startingDir)
    {
        if (dialogs.find(key) == dialogs.end())
        {
            dialogs[key] = FileDialogState();
        }

        FileDialogState &state = dialogs[key];
        state.key = key;
        state.title = title;
        state.type = type;
        state.startingDir = startingDir;
        state.callback = callback;
        state.open = true;

        if (!startingDir.empty() && std::filesystem::exists(startingDir))
        {
            loadDirectory(startingDir);
        }
        else
        {
            populateDrives();
        }
    }
    // dialogs/mui_file_dialog.cpp (Update to the render method)

    void FileDialog::render()
    {
        processLoadedPreviewsOnMainThread();

        for (auto &[key, dialog] : dialogs)
        {
            if (!dialog.open)
                continue;

            // Sync callbacks for this specific dialog instance
            bindCallbacks(dialog);
            bottomBar.isDirectoryMode = (dialog.type == FileDialogType::OpenDirectory);

            // Center the dialog on first use
            ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_FirstUseEver);

            if (ImGui::Begin(dialog.title.c_str(), &dialog.open, ImGuiWindowFlags_NoScrollbar))
            {

                // --- TOP BAR (Nav & Search) ---
                topBar.renderControl();
                ImGui::Separator();

                // --- MAIN CONTENT AREA (Sidebar + Grid) ---
                // We use a table to create a resizable split-pane effect
                static float sidebarWidth = 200.0f;
                if (ImGui::BeginTable("##split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY))
                {
                    ImGui::TableSetupColumn("Sidebar", ImGuiTableColumnFlags_WidthFixed, sidebarWidth);
                    ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow();

                    // Column 0: Navigation Tree
                    ImGui::TableSetColumnIndex(0);
                    ImGui::BeginChild("##tree_nav");
                    sidebar.renderControl();
                    ImGui::EndChild();

                    // Column 1: The Icon/File Grid
                    ImGui::TableSetColumnIndex(1);
                    ImGui::BeginChild("##grid_view");
                    mainGrid.renderControl();
                    ImGui::EndChild();

                    ImGui::EndTable();
                }

                // --- BOTTOM BAR (Filename & Actions) ---
                ImGui::Separator();
                bottomBar.renderControl();
            }
            ImGui::End();
        }
    }

} // namespace mui
