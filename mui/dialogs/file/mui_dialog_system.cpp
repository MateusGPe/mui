// src/ImFileDialog/ImFileDialog_System.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "mui_dialog.h"
#include <algorithm>
#include <imgui.h>

namespace mui_dlg
{
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
		m_fixDrivePath(m_currentDirectory);

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

		if (m_currentDirectory.string() == "Quick Access")
		{
			for (auto &node : m_treeCache)
			{
				if (node->Path == m_currentDirectory)
					for (auto &c : node->Children)
						m_content.push_back(FileData(c->Path));
			}
		}
		else if (m_currentDirectory.string() == "This PC")
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
						std::string filename = info.Path.string();

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
								std::string extension = info.Path.extension().string();
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
					std::string lName = left.Path.string();
					std::string rName = right.Path.string();

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
