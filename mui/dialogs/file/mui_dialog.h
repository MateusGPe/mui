// include/ImFileDialog/ImFileDialog.h
#pragma once
#include "../../layouts/box.hpp"
#include "../../widgets/breadcrumb.hpp"
#include "../../widgets/button.hpp"
#include "../../widgets/combobox.hpp"
#include "../../widgets/entry.hpp"
#include "../../widgets/iconbutton.hpp"
#include "mui_models.h"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#define IFD_DIALOG_FILE 0
#define IFD_DIALOG_DIRECTORY 1
#define IFD_DIALOG_SAVE 2
#include <queue>

namespace mui_dlg
{
  class FileDialog
  {
  public:
    // Maintaining backward compatibility with aliases
    using FileTreeNode = mui_dlg::FileTreeNode;
    using FileData = mui_dlg::FileData;

    static inline FileDialog &Instance()
    {
      static FileDialog ret;
      return ret;
    }

    FileDialog();
    ~FileDialog();

    bool Save(const std::string &key, const std::string &title,
              const std::string &filter, const std::string &startingDir = "");

    bool Open(const std::string &key, const std::string &title,
              const std::string &filter, bool isMultiselect = false,
              const std::string &startingDir = "");

    bool IsDone(const std::string &key);

    inline bool HasResult() { return m_result.size(); }
    inline const std::filesystem::path &GetResult() { return m_result[0]; }
    inline const std::vector<std::filesystem::path> &GetResults()
    {
      return m_result;
    }

    void Close();

    void RemoveFavorite(const std::string &path);
    void AddFavorite(const std::string &path);
    inline const std::vector<std::string> &GetFavorites() { return m_favorites; }

    inline void SetZoom(float z)
    {
      m_zoom = std::min<float>(25.0f, std::max<float>(1.0f, z));
      m_refreshIconPreview();
    }
    inline float GetZoom() { return m_zoom; }

    std::function<void *(uint8_t *, int, int, char)>
        CreateTexture; // char -> fmt -> { 0 = BGRA, 1 = RGBA }
    std::function<void(void *)> DeleteTexture;

  private:
    struct PreviewResult
    {
      std::filesystem::path Path;
      unsigned char *IconPreviewData;
      int IconPreviewWidth, IconPreviewHeight;
    };

  private:
    // mui controls
    mui::HBoxPtr m_toolbar;
    mui::IconButtonPtr m_backButton;
    mui::IconButtonPtr m_forwardButton;
    mui::IconButtonPtr m_upButton;
    mui::BreadcrumbBarPtr m_pathBox;
    mui::IconButtonPtr m_favoriteButton;
    mui::EntryPtr m_searchBox;

    mui::HBoxPtr m_bottomToolbar;
    mui::EntryPtr m_fileNameInput;
    mui::ComboBoxPtr m_filterCombo;
    mui::ButtonPtr m_cancelButton;
    mui::ButtonPtr m_okButton;

    std::string m_currentKey;
    std::string m_currentTitle;
    std::filesystem::path m_currentDirectory;
    bool m_isMultiselect;
    bool m_isOpen;
    uint8_t m_type;
    char m_inputTextbox[1024];
    char m_pathBuffer[1024];
    char m_newEntryBuffer[1024];
    char m_searchBuffer[128];
    std::vector<std::string> m_favorites;
    bool m_calledOpenPopup;
    std::stack<std::filesystem::path> m_backHistory, m_forwardHistory;
    float m_zoom;

    std::vector<std::filesystem::path> m_selections;
    int m_selectedFileItem;
    void m_select(const std::filesystem::path &path, bool isCtrlDown = false);

    std::vector<std::filesystem::path> m_result;
    bool m_finalize(const std::string &filename = "");

    std::string m_filter;
    std::vector<std::vector<std::string>> m_filterExtensions;
    size_t m_filterSelection;
    void m_parseFilter(const std::string &filter);

    std::vector<int> m_iconIndices;
    std::vector<std::string>
        m_iconFilepaths; // m_iconIndices[x] <-> m_iconFilepaths[x]
    std::unordered_map<std::string, void *> m_icons;
    void m_initSystemFavorites();
    void m_fixDrivePath(std::filesystem::path &p);
    void *m_getIcon(const std::filesystem::path &path, bool isDirectory);
    void m_clearIcons();
    void m_refreshIconPreview();
    void m_clearIconPreview();

    // Async Preview Loader Components
    std::thread *m_previewLoader;
    std::atomic<bool> m_previewLoaderRunning;
    std::atomic<bool> m_previewThreadExit{false};
    std::condition_variable m_previewCv;
    std::mutex m_previewQueueMutex;
    std::vector<std::filesystem::path> m_previewQueue;
    std::queue<PreviewResult> m_previewResults;
    std::mutex m_previewResultsMutex;
    void m_startPreviewLoaderIfNeeded();
    void m_stopPreviewLoader();
    void m_loadPreview();

    std::vector<FileTreeNode *> m_treeCache;
    void m_clearTree(FileTreeNode *node);
    void m_renderTree(FileTreeNode *node);

    unsigned int m_sortColumn;
    unsigned int m_sortDirection;
    std::vector<FileData> m_content;
    std::recursive_mutex m_contentMutex;

    void m_setDirectory(const std::filesystem::path &p, bool addHistory = true);
    void m_sortContent(unsigned int column, unsigned int sortDirection);

    void m_processPreviewResults();
    void m_renderContent();
    void m_renderPopups();
    void m_renderFileDialog();
  };
} // namespace mui_dlg