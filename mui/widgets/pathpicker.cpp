// widgets/pathpicker.cpp
#include "pathpicker.hpp"
#include "../core/app.hpp"
#include "../core/scoped.hpp"
#include "../dialogs/dialogs.hpp"
#include "../dialogs/file/mui_dialog.h"
#include "../include/IconsFontAwesome6.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <imgui.h>
#include <sstream>

#ifndef ICON_FA_FOLDER_OPEN
#define ICON_FA_FOLDER_OPEN "\xef\x81\xbc"
#endif
#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "\xef\x81\xbb"
#endif
#ifndef ICON_FA_FOLDER_PLUS
#define ICON_FA_FOLDER_PLUS "\xef\x99\x9e"
#endif
#ifndef ICON_FA_FILE
#define ICON_FA_FILE "\xef\x85\x9b"
#endif
#ifndef ICON_FA_ARROW_UP
#define ICON_FA_ARROW_UP "\xef\x81\xa2"
#endif
#ifndef ICON_FA_ARROW_ROTATE_RIGHT
#define ICON_FA_ARROW_ROTATE_RIGHT "\xef\x80\x9e"
#endif
#ifndef ICON_FA_STAR
#define ICON_FA_STAR "\xef\x80\x85"
#endif

namespace mui
{
    // Portability Helper adapted from mui_dialog's m_fixDrivePath
    static void fixDrivePath(std::filesystem::path &p)
    {
#ifdef _WIN32
        std::string s = p.string();
        // Drives don't work well in std::filesystem without the trailing backslash
        if (s.size() == 2 && s[1] == ':')
        {
            p = std::filesystem::path(s + "\\");
        }
#endif
    }

    PathPicker::PathPicker()
    {
        App::assertMainThread();
        m_breadcrumb = BreadcrumbBar::create("");
        m_fileCombo = ComboBox::create();

        m_fileCombo->onChanged(
            [this](int idx)
            {
                if (idx >= 0)
                {
                    std::string selectedText = m_fileCombo->getText();
                    if (selectedText.empty())
                        return;

                    bool is_dir = selectedText.rfind(ICON_FA_FOLDER, 0) == 0;
                    std::string name;

                    if (is_dir)
                        name = selectedText.substr(strlen(ICON_FA_FOLDER) + 1);
                    else
                        name = selectedText.substr(strlen(ICON_FA_FILE) + 1);

                    std::filesystem::path dir = m_breadcrumb->getPath();
                    fixDrivePath(dir);
                    if (!std::filesystem::is_directory(dir))
                        dir = dir.parent_path();
                    std::string fullPath = (dir / name).string();

                    App::queueMain(
                        [this, is_dir, fullPath, name]()
                        {
                            if (is_dir)
                            {
                                setPath(fullPath);
                                onPathChangedSignal(fullPath);
                            }
                            else
                            {
                                m_path = fullPath;
                                if (m_fileEntry)
                                    m_fileEntry->setText(name);
                                onPathChangedSignal(fullPath);
                            }
                        });
                }
            });

        m_fileEntry = Entry::create()
                          ->setHint("File Name")
                          ->onChanged(
                              [this]()
                              {
                                  std::filesystem::path dir = m_breadcrumb->getPath();
                                  fixDrivePath(dir);
                                  std::error_code ec;
                                  if (!std::filesystem::is_directory(dir, ec))
                                  {
                                      dir = dir.parent_path();
                                  }
                                  std::string fullPath = (dir / m_fileEntry->getText()).string();
                                  m_path = fullPath;
                                  onPathChangedSignal(fullPath);
                              });

        m_upButton = IconButton::create(ICON_FA_ARROW_UP)
                         ->onClick(
                             [this]()
                             {
                                 std::filesystem::path p(m_breadcrumb->getPath());
                                 fixDrivePath(p);
                                 if (p.has_parent_path() && p.parent_path() != p)
                                 {
                                     auto newPath = p.parent_path().string();
                                     setPath(newPath);
                                     onPathChangedSignal(newPath);
                                 }
                             });

        m_refreshButton = IconButton::create(
                              ICON_FA_ARROW_ROTATE_RIGHT)
                              ->onClick([this]()
                                        { updateFileList(); });

        m_favoritesButton = IconButton::create(
                                ICON_FA_STAR)
                                ->onClick([this]()
                                          { m_openFavoritesPopup = true; });

        m_newFolderButton = IconButton::create(
                                ICON_FA_FOLDER_PLUS)
                                ->onClick([this]()
                                          { openNewFolderPopup(); });

        m_filterCombo = ComboBox::create()
                            ->onChanged(
                                [this](int idx)
                                {
                                    if (idx != m_filterSelection)
                                    {
                                        m_filterSelection = idx;
                                        App::queueMain(
                                            [this]()
                                            {
                                                updateFileList();
                                            });
                                    }
                                });
        m_filterCombo->setMinWidth(150.f * App::getDpiScale());

        m_browseButton =
            IconButton::create(ICON_FA_FOLDER_OPEN)
                ->onClick(
                    [this]()
                    {
                        auto onSuccess = [this](const std::string &p)
                        {
                            setPath(p);
                            onPathChangedSignal(p);
                        };

                        auto onCancel = []() {};

                        if (m_mode == PathPickerMode::File)
                        {
                            Dialogs::openFile("Select File",
                                              m_filter.empty() ? "All Files {*.*}"
                                                               : m_filter,
                                              onSuccess, onCancel);
                        }
                        else if (m_mode == PathPickerMode::SaveFile)
                        {
                            Dialogs::saveFile("Save File",
                                              m_filter.empty() ? "All Files {*.*} "
                                                               : m_filter,
                                              onSuccess, onCancel);
                        }
                        else if (m_mode == PathPickerMode::Folder)
                        {
                            Dialogs::openFile("Select Folder", "", onSuccess,
                                              onCancel);
                        }
                    });

        m_breadcrumb->onPathNavigated(
            [this](const std::string &p)
            {
                std::error_code ec;
                std::filesystem::path pathObj(p);
                fixDrivePath(pathObj);
                std::string new_path = pathObj.string();

                if (m_mode != PathPickerMode::Folder)
                {
                    // If it's a directory, try to preserve the existing filename
                    if (std::filesystem::is_directory(pathObj, ec))
                    {
                        std::filesystem::path current(m_path);
                        if (!current.empty() && !std::filesystem::is_directory(current, ec))
                        {
                            new_path = (pathObj / current.filename()).string();
                        }
                    }
                    // If it's NOT a directory, it means the user manually pasted a full
                    // path to a specific file. We will just pass that through directly.
                }

                // Route through setPath to cleanly separate the file and directory values
                setPath(new_path);
                onPathChangedSignal(m_path);
            });

        rebuildLayout();
    }

    void PathPicker::openNewFolderPopup()
    {
        if (m_breadcrumb->getPath().empty())
            return;

        Dialogs::msgBoxInput(
            "New Folder",
            "Enter new folder name:",
            "",
            [this](const std::string &name)
            {
                if (!name.empty() && !m_breadcrumb->getPath().empty())
                {
                    std::filesystem::path baseDir(m_breadcrumb->getPath());
                    fixDrivePath(baseDir);
                    std::filesystem::path new_dir = baseDir / name;
                    std::error_code ec;
                    if (std::filesystem::create_directory(new_dir, ec))
                    {
                        updateFileList();
                    }
                    else
                    {
                        Dialogs::msgBoxError("Error", "Could not create directory:\n" +
                                                          ec.message());
                    }
                }
            });
    }

    void PathPicker::rebuildLayout()
    {
        m_layout = HBox::create()->setPadded(false);

        // Append conditionally based on feature flags
        if (m_showUp)
            m_layout->append(m_upButton);

        if (m_showRefresh)
            m_layout->append(m_refreshButton);

        if (m_showFavorites)
            m_layout->append(m_favoritesButton);

        if (m_showBreadcrumb)
            m_layout->append(m_breadcrumb, true);

        if (m_mode != PathPickerMode::Folder)
        {
            if (m_showFile)
            {
                if (m_mode == PathPickerMode::SaveFile)
                {
                    m_layout->append(m_fileEntry, true);
                }
                else
                {
                    m_layout->append(m_fileCombo, true);
                }
            }

            if (m_showFilter && !m_filters.empty())
            {
                m_layout->append(m_filterCombo);
            }
        }

        if (m_showNewFolder)
            m_layout->append(m_newFolderButton);

        if (m_showBrowse)
            m_layout->append(m_browseButton);
    }

    void PathPicker::parseFilter(const std::string &filter)
    {
        bool was_empty = m_filters.empty();
        m_filters.clear();
        m_filterCombo->clear();
        if (filter.empty())
        {
            if (was_empty != m_filters.empty())
            {
                rebuildLayout();
            }
            return;
        }

        size_t current_pos = 0;
        while (current_pos < filter.length())
        {
            size_t brace_open = filter.find('{', current_pos);
            if (brace_open == std::string::npos)
                break;

            size_t brace_close = filter.find('}', brace_open);
            if (brace_close == std::string::npos)
                break;

            std::string name = filter.substr(current_pos, brace_open - current_pos);
            name.erase(0, name.find_first_not_of(" \t\n\r"));
            if (!name.empty())
            {
                name.erase(name.find_last_not_of(" \t\n\r") + 1);
            }

            std::string exts_str =
                filter.substr(brace_open + 1, brace_close - brace_open - 1);
            std::vector<std::string> exts;
            std::stringstream ss(exts_str);
            std::string ext;
            while (std::getline(ss, ext, ','))
            {
                ext.erase(0, ext.find_first_not_of(" \t\n\r"));
                if (!ext.empty())
                {
                    ext.erase(ext.find_last_not_of(" \t\n\r") + 1);
                    exts.push_back(ext);
                }
            }
            m_filters.push_back({name, exts});
            m_filterCombo->append(name);

            current_pos = brace_close + 1;
            if (current_pos < filter.length() && filter[current_pos] == ',')
            {
                current_pos++;
            }
        }

        if (was_empty != m_filters.empty())
        {
            rebuildLayout();
        }
    }

    void PathPicker::updateFileList()
    {
        if (m_mode == PathPickerMode::Folder)
            return;

        m_fileCombo->clear();

        std::error_code ec;
        std::filesystem::path dir = m_breadcrumb->getPath();
        fixDrivePath(dir);
        if (!std::filesystem::is_directory(dir, ec))
            return;

        std::string currentFilename;
        if (!std::filesystem::is_directory(m_path, ec))
        {
            currentFilename = std::filesystem::path(m_path).filename().string();
        }

        std::vector<std::filesystem::path> directories;
        std::vector<std::filesystem::path> files;

        try
        {
            auto it = std::filesystem::directory_iterator(
                dir, std::filesystem::directory_options::skip_permission_denied, ec);
            if (!ec)
            {
                auto end = std::filesystem::directory_iterator();
                while (it != end)
                {
                    try
                    {
                        const auto &entry = *it;
                        if (entry.is_directory(ec))
                            directories.push_back(entry.path());
                        else if (entry.is_regular_file(ec))
                            files.push_back(entry.path());
                    }
                    catch (...)
                    {
                        /* Suppress individual read failures */
                    }

                    it.increment(ec);
                    if (ec)
                    {
                        ec.clear();
                    }
                }
            }
        }
        catch (...)
        {
            /* Suppress outright catastrophic directory open failures */
        }

        auto sort_by_name = [](const auto &a, const auto &b)
        {
            return a.filename().string() < b.filename().string();
        };
        std::sort(directories.begin(), directories.end(), sort_by_name);
        std::sort(files.begin(), files.end(), sort_by_name);

        int current_item_index = 0;
        int targetIdx = -1;

        for (const auto &p : directories)
        {
            m_fileCombo->append(std::string(ICON_FA_FOLDER) + " " +
                                p.filename().string());
            current_item_index++;
        }

        std::vector<std::string> lower_exts;
        bool filter_active = m_filterSelection >= 0 &&
                             m_filterSelection < static_cast<int>(m_filters.size());
        bool is_wildcard = false;

        if (filter_active)
        {
            const auto &exts = m_filters[m_filterSelection].second;
            for (const auto &e : exts)
            {
                if (e == "*.*" || e == ".*" || e == "*")
                {
                    is_wildcard = true;
                    break;
                }
                std::string lower_e = e;
                if (lower_e.length() > 0 && lower_e[0] == '*')
                    lower_e = lower_e.substr(1);

                std::transform(lower_e.begin(), lower_e.end(), lower_e.begin(),
                               [](unsigned char c)
                               {
                                   return std::tolower(c);
                               });
                lower_exts.push_back(lower_e);
            }
        }

        for (const auto &p : files)
        {
            std::string fname = p.filename().string();

            if (filter_active && !is_wildcard && !lower_exts.empty())
            {
                std::string file_ext = p.extension().string();
                std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(),
                               [](unsigned char c)
                               {
                                   return std::tolower(c);
                               });

                bool found = false;
                for (const auto &e : lower_exts)
                {
                    if (e == file_ext)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    continue;
            }

            m_fileCombo->append(std::string(ICON_FA_FILE) + " " + fname);
            if (!currentFilename.empty() && fname == currentFilename)
                targetIdx = current_item_index;
            current_item_index++;
        }

        if (targetIdx == -1 && !currentFilename.empty() &&
            m_mode == PathPickerMode::File)
        {
            m_fileCombo->append(std::string(ICON_FA_FILE) + " " + currentFilename);
            targetIdx = current_item_index;
        }

        if (targetIdx >= 0)
        {
            m_fileCombo->setSelectedIndex(targetIdx);
        }
    }

    void PathPicker::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);

        std::filesystem::path p(m_breadcrumb->getPath());
        fixDrivePath(p);
        m_upButton->setEnabled(p.has_parent_path() && p.parent_path() != p);


        if (m_mode == PathPickerMode::File)
        {
            std::string preview;
            int s_idx = m_fileCombo->getSelectedIndex();
            if (s_idx >= 0)
            {
                std::string selectedText = m_fileCombo->getText();
                if (!selectedText.empty())
                {
                    bool is_dir = selectedText.rfind(ICON_FA_FOLDER, 0) == 0;
                    size_t offset =
                        is_dir ? strlen(ICON_FA_FOLDER) + 1 : strlen(ICON_FA_FILE) + 1;
                    if (selectedText.length() > offset)
                        preview = selectedText.substr(offset);
                }
            }
            else if (!m_path.empty() && !std::filesystem::is_directory(m_path))
            {
                preview = std::filesystem::path(m_path).filename().string();
            }
            m_fileCombo->setCustomPreviewValue(preview);
        }

        ImGui::BeginDisabled(!enabled);

        if (spanAvailWidth)
        {
            m_layout->setSpanAvailWidth(true);
        }
        else if (width > 0)
        {
            m_layout->setWidth(width);
        }
        else if (useContainerWidth)
        {
            m_layout->setUseContainerWidth(true);
        }

        m_layout->render();

        if (m_openFavoritesPopup)
        {
            ImGui::OpenPopup("##pathpicker_favorites");
            m_openFavoritesPopup = false;
        }

        if (ImGui::BeginPopup("##pathpicker_favorites"))
        {
            auto &fd = mui_dlg::FileDialog::Instance();
            for (const auto &fav : fd.GetFavorites())
            {
                if (ImGui::MenuItem(fav.c_str()))
                {
                    setPath(fav);
                    onPathChangedSignal(fav);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Add Current to Favorites"))
            {
                fd.AddFavorite(m_breadcrumb->getPath());
            }
            ImGui::EndPopup();
        }

        renderTooltip();
        ImGui::EndDisabled();
    }

    std::string PathPicker::getPath() const { return m_path; }

    PathPickerPtr PathPicker::setPath(const std::string &path)
    {
        std::filesystem::path p(path);
        fixDrivePath(p);
        std::string fixedPath = p.string();

        if (m_path == fixedPath)
            return self();

        m_path = fixedPath;
        std::error_code ec;

        if (m_mode == PathPickerMode::Folder)
        {
            m_breadcrumb->setPath(fixedPath);
        }
        else
        {
            if (std::filesystem::is_directory(p, ec))
            {
                m_breadcrumb->setPath(fixedPath);
                if (m_mode == PathPickerMode::SaveFile && m_fileEntry)
                {
                    m_fileEntry->setText("");
                }
            }
            else
            {
                m_breadcrumb->setPath(p.parent_path().string());
                if (m_mode == PathPickerMode::SaveFile && m_fileEntry)
                {
                    m_fileEntry->setText(p.filename().string());
                }
            }
            updateFileList();
        }
        return self();
    }

    PathPickerPtr PathPicker::setMode(PathPickerMode mode)
    {
        if (m_mode != mode)
        {
            m_mode = mode;
            rebuildLayout();
            if (m_mode != PathPickerMode::Folder)
            {
                updateFileList();
            }
        }
        return self();
    }

    PathPickerPtr PathPicker::setFilter(const std::string &filter)
    {
        if (m_filter != filter)
        {
            m_filter = filter;
            parseFilter(filter);
            m_filterSelection = 0;
            m_filterCombo->setSelectedIndex(0);
            updateFileList();
        }
        return self();
    }

    // Setting implementations for toggles
    PathPickerPtr PathPicker::setShowFavorites(bool show)
    {
        if (m_showFavorites != show)
        {
            m_showFavorites = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr PathPicker::setShowUp(bool show)
    {
        if (m_showUp != show)
        {
            m_showUp = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr PathPicker::setShowNewFolder(bool show)
    {
        if (m_showNewFolder != show)
        {
            m_showNewFolder = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr PathPicker::setShowRefresh(bool show)
    {
        if (m_showRefresh != show)
        {
            m_showRefresh = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr PathPicker::setShowBreadcrumb(bool show)
    {
        if (m_showBreadcrumb != show)
        {
            m_showBreadcrumb = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr PathPicker::setShowFile(bool show)
    {
        if (m_showFile != show)
        {
            m_showFile = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr PathPicker::setShowFilter(bool show)
    {
        if (m_showFilter != show)
        {
            m_showFilter = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr PathPicker::setShowBrowse(bool show)
    {
        if (m_showBrowse != show)
        {
            m_showBrowse = show;
            rebuildLayout();
        }
        return self();
    }

    PathPickerPtr
    PathPicker::bind(std::shared_ptr<Observable<std::string>> observable)
    {
        setPath(observable->get());
        m_connections.push_back(
            observable
                ->onValueChanged
                .connect(
                    [this](const std::string &val)
                    {
                        mui::App::queueMain(
                            [this, val]()
                            {
                                this->setPath(val);
                            });
                    }));
        m_connections.push_back(onPathChangedSignal.connect(
            [observable](const std::string &val)
            {
                observable->set(val);
            }));
        return self();
    }

    PathPickerPtr
    PathPicker::onPathChanged(std::function<void(const std::string &)> cb)
    {
        if (cb)
            m_connections.push_back(onPathChangedSignal.connect(std::move(cb)));
        return self();
    }
} // namespace mui