// widgets/pathpicker.hpp
#pragma once
#include "control.hpp"
#include "breadcrumb.hpp"
#include "iconbutton.hpp"
#include "combobox.hpp"
#include "../layouts/box.hpp"
#include "entry.hpp"
#include "../core/observable.hpp"
#include "../core/signal.hpp"
#include <string>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <thread>
#include <atomic>

#ifdef _WIN32
#define INITIAL_PATH "C:\\"
#else
#define INITIAL_PATH "/"
#endif

namespace mui
{
    enum class PathPickerMode
    {
        File,
        Folder,
        SaveFile
    };

    class PathPicker;
    using PathPickerPtr = std::shared_ptr<PathPicker>;

    class PathPicker : public Control<PathPicker>
    {
    protected:
        std::string getTypeName() const override { return "PathPicker"; }
        std::string m_path;
        PathPickerMode m_mode = PathPickerMode::File;
        std::string m_filter;
        bool m_openFavoritesPopup = false;

        // Feature toggles
        bool m_showFavorites = true;
        bool m_showUp = true;
        bool m_showNewFolder = true;
        bool m_showRefresh = true;
        bool m_showBreadcrumb = true;
        bool m_showFile = true;
        bool m_showFilter = true;
        bool m_showBrowse = true;

        BreadcrumbBarPtr m_breadcrumb;
        IconButtonPtr m_upButton;
        IconButtonPtr m_refreshButton;
        IconButtonPtr m_favoritesButton;
        IconButtonPtr m_newFolderButton;
        ComboBoxPtr m_fileCombo;
        EntryPtr m_fileEntry;
        ComboBoxPtr m_filterCombo;
        IconButtonPtr m_browseButton;
        HBoxPtr m_layout;

        std::vector<std::pair<std::string, std::vector<std::string>>> m_filters;
        int m_filterSelection = 0;

        PathPicker();

        void openNewFolderPopup();
        void rebuildLayout();
        void updateFileList();
        void parseFilter(const std::string &filter);

    public:
        mui::Signal<std::string> onPathChangedSignal;

        static PathPickerPtr create(const std::string &initialPath = INITIAL_PATH)
        {
            auto p = std::shared_ptr<PathPicker>(new PathPicker());
            return p->setPath(initialPath);
        }

        void renderControl() override;

        std::string getPath() const;
        PathPickerPtr setPath(const std::string &path);
        PathPickerPtr setMode(PathPickerMode mode);
        PathPickerPtr setFilter(const std::string &filter);

        // Feature toggles setters
        PathPickerPtr setShowFavorites(bool show);
        PathPickerPtr setShowUp(bool show);
        PathPickerPtr setShowNewFolder(bool show);
        PathPickerPtr setShowRefresh(bool show);
        PathPickerPtr setShowBreadcrumb(bool show);
        PathPickerPtr setShowFile(bool show);
        PathPickerPtr setShowFilter(bool show);
        PathPickerPtr setShowBrowse(bool show);

        PathPickerPtr bind(std::shared_ptr<Observable<std::string>> observable);
        PathPickerPtr onPathChanged(std::function<void(const std::string &)> cb);

        BreadcrumbBarPtr getBreadcrumb() const { return m_breadcrumb; }
        IconButtonPtr getUpButton() const { return m_upButton; }
        ComboBoxPtr getFileCombo() const { return m_fileCombo; }
        EntryPtr getFileEntry() const { return m_fileEntry; }
        ComboBoxPtr getFilterCombo() const { return m_filterCombo; }
        IconButtonPtr getBrowseButton() const { return m_browseButton; }
    };
} // namespace mui