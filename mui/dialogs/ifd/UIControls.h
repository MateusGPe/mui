// include/ImFileDialog/UIControls.h
#pragma once
#include <imgui.h>
#include <filesystem>

namespace ifd
{
	bool FolderNode(const char *label, ImTextureID icon, bool &clicked);
	bool FileNode(const char *label, ImTextureID icon);
	bool PathBox(const char *label, std::filesystem::path &path, char *pathBuffer, ImVec2 size_arg);
	bool FavoriteButton(const char *label, bool isFavorite);
	bool FileIcon(const char *label, bool isSelected, ImTextureID icon, bool isDirectory, ImVec2 size, bool hasPreview, int previewWidth, int previewHeight);
}
