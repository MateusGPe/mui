// include/ImFileDialog/Models.h
#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <ctime>

namespace ifd
{
	class FileTreeNode
	{
	public:
		FileTreeNode(const std::string &path)
		{
			Path = std::filesystem::u8path(path);
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
		uint8_t *IconPreviewData;
		int IconPreviewWidth, IconPreviewHeight;
	};
}
