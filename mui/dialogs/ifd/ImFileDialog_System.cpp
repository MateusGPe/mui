// src/ImFileDialog/ImFileDialog_System.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ImFileDialog.h"
#include <algorithm>
#include <imgui.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#endif

namespace ifd
{
	void *FileDialog::m_getIcon(const std::filesystem::path &path, bool isDirectory)
	{
#ifdef _WIN32
		if (m_icons.count(path.u8string()) > 0)
			return m_icons[path.u8string()];

		std::string pathU8 = path.u8string();

		std::error_code ec;
		m_icons[pathU8] = nullptr;

		DWORD attrs = 0;
		UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
		if (!std::filesystem::exists(path, ec))
		{
			flags |= SHGFI_USEFILEATTRIBUTES;
			attrs = FILE_ATTRIBUTE_DIRECTORY;
		}

		SHFILEINFOW fileInfo = {0};
		std::wstring pathW = path.wstring();
		for (int i = 0; i < pathW.size(); i++)
			if (pathW[i] == '/')
				pathW[i] = '\\';
		SHGetFileInfoW(pathW.c_str(), attrs, &fileInfo, sizeof(SHFILEINFOW), flags);

		if (fileInfo.hIcon == nullptr)
			return nullptr;

		// check if icon is already loaded
		auto itr = std::find(m_iconIndices.begin(), m_iconIndices.end(), fileInfo.iIcon);
		if (itr != m_iconIndices.end())
		{
			const std::string &existingIconFilepath = m_iconFilepaths[itr - m_iconIndices.begin()];
			m_icons[pathU8] = m_icons[existingIconFilepath];
			return m_icons[pathU8];
		}

		m_iconIndices.push_back(fileInfo.iIcon);
		m_iconFilepaths.push_back(pathU8);

		ICONINFO iconInfo = {0};
		GetIconInfo(fileInfo.hIcon, &iconInfo);

		if (iconInfo.hbmColor == nullptr)
			return nullptr;

		DIBSECTION ds;
		GetObject(iconInfo.hbmColor, sizeof(ds), &ds);
		int byteSize = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

		if (byteSize == 0)
			return nullptr;

		uint8_t *data = (uint8_t *)malloc(byteSize);
		GetBitmapBits(iconInfo.hbmColor, byteSize, data);

		m_icons[pathU8] = this->CreateTexture(data, ds.dsBm.bmWidth, ds.dsBm.bmHeight, 0);

		free(data);

		return m_icons[pathU8];
#else
		// Naturally fall back to FontAwesome
		return nullptr;
#endif
	}

	void FileDialog::m_clearIcons()
	{
		std::vector<unsigned int> deletedIcons;

		// delete textures
		for (auto &icon : m_icons)
		{
			unsigned int ptr = (unsigned int)((uintptr_t)icon.second);
			if (std::count(deletedIcons.begin(), deletedIcons.end(), ptr)) // skip duplicates
				continue;

			deletedIcons.push_back(ptr);
			DeleteTexture(icon.second);
		}
		m_iconFilepaths.clear();
		m_iconIndices.clear();
		m_icons.clear();
	}

	void FileDialog::m_setDirectory(const std::filesystem::path &p, bool addHistory)
	{
		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);

		bool isSameDir = m_currentDirectory == p;

		if (addHistory && !isSameDir)
			m_backHistory.push(m_currentDirectory);

		m_currentDirectory = p;
#ifdef _WIN32
		// drives don't work well without the backslash symbol
		if (p.u8string().size() == 2 && p.u8string()[1] == ':')
			m_currentDirectory = std::filesystem::u8path(p.u8string() + "\\");
#endif

		m_clearIconPreview();
		m_content.clear(); // p == "" after this line, due to reference
		m_selectedFileItem = -1;

		if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE)
			m_inputTextbox[0] = 0;
		m_selections.clear();

		if (!isSameDir)
		{
			m_searchBuffer[0] = 0;
			m_clearIcons();
		}

		if (m_currentDirectory.u8string() == "Quick Access")
		{
			for (auto &node : m_treeCache)
			{
				if (node->Path == m_currentDirectory)
					for (auto &c : node->Children)
						m_content.push_back(FileData(c->Path));
			}
		}
		else if (m_currentDirectory.u8string() == "This PC")
		{
			for (auto &node : m_treeCache)
			{
				if (node->Path == m_currentDirectory)
					for (auto &c : node->Children)
						m_content.push_back(FileData(c->Path));
			}
		}
		else
		{
			std::error_code ec;
			if (std::filesystem::exists(m_currentDirectory, ec))
				for (const auto &entry : std::filesystem::directory_iterator(m_currentDirectory, ec))
				{
					FileData info(entry.path());

					// skip files when IFD_DIALOG_DIRECTORY
					if (!info.IsDirectory && m_type == IFD_DIALOG_DIRECTORY)
						continue;

					// check if filename matches search query
					if (m_searchBuffer[0])
					{
						std::string filename = info.Path.u8string();

						std::string filenameSearch = filename;
						std::string query(m_searchBuffer);
						std::transform(filenameSearch.begin(), filenameSearch.end(), filenameSearch.begin(), ::tolower);
						std::transform(query.begin(), query.end(), query.begin(), ::tolower);

						if (filenameSearch.find(query, 0) == std::string::npos)
							continue;
					}

					// check if extension matches
					if (!info.IsDirectory && m_type != IFD_DIALOG_DIRECTORY)
					{
						if (m_filterSelection < m_filterExtensions.size())
						{
							const auto &exts = m_filterExtensions[m_filterSelection];
							if (exts.size() > 0)
							{
								std::string extension = info.Path.extension().u8string();
								bool isWildcard = (std::count(exts.begin(), exts.end(), "*.*") > 0);

								// extension not found? skip
								// if it's not a wildcard filter and the extension doesn't match, skip it
								if (!isWildcard && std::count(exts.begin(), exts.end(), extension) == 0)
									continue;
							}
						}
					}

					m_content.push_back(info);
				}
		}

		m_sortContent(m_sortColumn, m_sortDirection);
		m_refreshIconPreview();
	}

	void FileDialog::m_sortContent(unsigned int column, unsigned int sortDirection)
	{
		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);

		// 0 -> name, 1 -> date, 2 -> size
		m_sortColumn = column;
		m_sortDirection = sortDirection;

		// split into directories and files
		std::partition(m_content.begin(), m_content.end(), [](const FileData &data)
					   { return data.IsDirectory; });

		if (m_content.size() > 0)
		{
			// find where the file list starts
			size_t fileIndex = 0;
			for (; fileIndex < m_content.size(); fileIndex++)
				if (!m_content[fileIndex].IsDirectory)
					break;

			// compare function
			auto compareFn = [column, sortDirection](const FileData &left, const FileData &right) -> bool
			{
				// name
				if (column == 0)
				{
					std::string lName = left.Path.u8string();
					std::string rName = right.Path.u8string();

					std::transform(lName.begin(), lName.end(), lName.begin(), ::tolower);
					std::transform(rName.begin(), rName.end(), rName.begin(), ::tolower);

					int comp = lName.compare(rName);

					if (sortDirection == ImGuiSortDirection_Ascending)
						return comp < 0;
					return comp > 0;
				}
				// date
				else if (column == 1)
				{
					if (sortDirection == ImGuiSortDirection_Ascending)
						return left.DateModified < right.DateModified;
					else
						return left.DateModified > right.DateModified;
				}
				// size
				else if (column == 2)
				{
					if (sortDirection == ImGuiSortDirection_Ascending)
						return left.Size < right.Size;
					else
						return left.Size > right.Size;
				}

				return false;
			};

			// sort the directories
			std::sort(m_content.begin(), m_content.begin() + fileIndex, compareFn);

			// sort the files
			std::sort(m_content.begin() + fileIndex, m_content.end(), compareFn);
		}
	}
}
