// include/ImFileDialog/Models.h
#pragma once
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

namespace mui_dlg
{
    class FileTreeNode
    {
    public:
        FileTreeNode(const std::string &path)
        {
            Path = std::filesystem::path(path.c_str());
            Read = false;
        }

        std::filesystem::path Path;
        bool Read;
        std::vector<FileTreeNode *> Children;
    };

    class FileData
    {
    public:
        FileData(const std::filesystem::path &path);

        std::filesystem::path Path;
        bool IsDirectory;
        size_t Size;
        time_t DateModified;

        bool HasIconPreview;
        void *IconPreview;
        int IconPreviewWidth, IconPreviewHeight;
    };
} // namespace mui_dlg
