// include/ImFileDialog/UIControls.h
#pragma once
#include <filesystem>
#include <imgui.h>

namespace mui_dlg
{
    bool FolderNode(const char *label, ImTextureID icon, bool &clicked);
    bool FileIcon(const char *label, bool isSelected, ImTextureID icon,
                  bool isDirectory, ImVec2 size, bool hasPreview, int previewWidth,
                  int previewHeight);
} // namespace mui_dlg
