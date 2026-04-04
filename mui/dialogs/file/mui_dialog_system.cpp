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
            if (std::count(deletedIcons.begin(), deletedIcons.end(),
                           ptr)) // skip duplicates
                continue;

            deletedIcons.push_back(ptr);
            DeleteTexture(icon.second);
        }
        m_iconFilepaths.clear();
        m_iconIndices.clear();
        m_icons.clear();
    }

    void FileDialog::m_setDirectory(const std::filesystem::path &p,
                                    bool addHistory)
    {
        std::string searchBufStr;
        int filterSelectionVal;
        std::vector<std::vector<std::string>> filterExts;
        std::vector<FileTreeNode *> treeCacheCopy;
        
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

            if (m_type == MUI_DIALOG_DIRECTORY || m_type == MUI_DIALOG_FILE)
                m_inputTextbox[0] = 0;
            m_selections.clear();

            if (!isSameDir)
            {
                m_searchBuffer[0] = 0;
                m_clearIcons();
            }

            searchBufStr = m_searchBuffer;
            filterSelectionVal = m_filterSelection;
            filterExts = m_filterExtensions;
            treeCacheCopy = m_treeCache; // Pointers are stable enough for this
        }

        std::thread([this, p = m_currentDirectory, searchBufStr, filterSelectionVal, filterExts, treeCacheCopy]() {
            std::vector<FileData> temp_content;
            temp_content.reserve(1024);

            if (p.string() == "Quick Access" || p.string() == "This PC")
            {
                for (auto &node : treeCacheCopy)
                {
                    if (node->Path == p)
                        for (auto &c : node->Children)
                            temp_content.push_back(FileData(c->Path));
                }
            }
            else
            {
                std::error_code ec;
                if (std::filesystem::exists(p, ec))
                {
                    try {
                        auto it = std::filesystem::directory_iterator(p, std::filesystem::directory_options::skip_permission_denied, ec);
                        if (!ec) {
                            auto end = std::filesystem::directory_iterator();
                            while (it != end) {
                                try {
                                    const auto &entry = *it;
                                    FileData info(entry.path());

                                    bool skip = false;
                                    if (!info.IsDirectory && m_type == MUI_DIALOG_DIRECTORY)
                                        skip = true;

                                    if (!skip && !searchBufStr.empty())
                                    {
                                        std::string filename = info.Path.string();
                                        std::string filenameSearch = filename;
                                        std::string query = searchBufStr;
                                        std::transform(filenameSearch.begin(), filenameSearch.end(),
                                                       filenameSearch.begin(), ::tolower);
                                        std::transform(query.begin(), query.end(), query.begin(), ::tolower);

                                        if (filenameSearch.find(query, 0) == std::string::npos)
                                            skip = true;
                                    }

                                    if (!skip && !info.IsDirectory && m_type != MUI_DIALOG_DIRECTORY)
                                    {
                                        if (filterSelectionVal >= 0 && filterSelectionVal < filterExts.size())
                                        {
                                            const auto &exts = filterExts[filterSelectionVal];
                                            if (exts.size() > 0)
                                            {
                                                std::string extension = info.Path.extension().string();
                                                bool isWildcard =
                                                    (std::count(exts.begin(), exts.end(), "*.*") > 0);

                                                if (!isWildcard &&
                                                    std::count(exts.begin(), exts.end(), extension) == 0)
                                                    skip = true;
                                            }
                                        }
                                    }

                                    if (!skip)
                                        temp_content.push_back(info);
                                } catch (...) {}
                                
                                it.increment(ec);
                                if (ec) ec.clear();
                            }
                        }
                    } catch (...) {}
                }
            }

            mui::App::queueMain([this, p, new_content = std::move(temp_content)]() mutable {
                std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
                if (m_currentDirectory != p) return; // Stale thread
                
                m_content = std::move(new_content);
                m_sortContent(m_sortColumn, m_sortDirection);
                m_refreshIconPreview();
            });
        }).detach();
    }

    void FileDialog::m_sortContent(unsigned int column,
                                   unsigned int sortDirection)
    {
        std::lock_guard<std::recursive_mutex> lock(m_contentMutex);

        // 0 -> name, 1 -> date, 2 -> size
        m_sortColumn = column;
        m_sortDirection = sortDirection;

        // split into directories and files
        std::partition(m_content.begin(), m_content.end(),
                       [](const FileData &data)
                       { return data.IsDirectory; });

        if (m_content.size() > 0)
        {
            // find where the file list starts
            size_t fileIndex = 0;
            for (; fileIndex < m_content.size(); fileIndex++)
                if (!m_content[fileIndex].IsDirectory)
                    break;

            // compare function
            auto compareFn = [column, sortDirection](const FileData &left,
                                                     const FileData &right) -> bool
            {
                // name
                if (column == 0)
                {
                    std::string lName = left.Path.filename().string();
                    std::string rName = right.Path.filename().string();

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
} // namespace mui_dlg
