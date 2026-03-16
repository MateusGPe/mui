#include "dialogs.hpp"
#include "mui_file_dialog.hpp" // Changed from ImFileDialog.h
#include "../core/app.hpp"

namespace mui
{
    void Dialogs::msgBox(const std::string &title, const std::string &message)
    {
        App::addMessageBox({title, message});
    }

    void Dialogs::msgBoxError(const std::string &title, const std::string &message)
    {
        // For now, same as msgBox. Could be styled differently in App::processMessageBoxes.
        App::addMessageBox({title, message});
    }

    void Dialogs::openFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir)
    {
        std::string key = "openfile##" + title;
        
        mui::FileDialog::Instance().Open(FileDialogType::OpenFile, key, title, [on_ok](const std::vector<std::string>& paths) {
            if (on_ok && !paths.empty()) {
                on_ok(paths[0]);
            }
        }, startingDir);
    }

    void Dialogs::openFiles(const std::string &title, const std::string &filter, std::function<void(const std::vector<std::string> &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir)
    {
        std::string key = "openfiles##" + title;
        
        mui::FileDialog::Instance().Open(FileDialogType::OpenFiles, key, title, on_ok, startingDir);
    }

    void Dialogs::saveFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir)
    {
        std::string key = "savefile##" + title;
        
        mui::FileDialog::Instance().Open(FileDialogType::SaveFile, key, title, [on_ok](const std::vector<std::string>& paths) {
            if (on_ok && !paths.empty()) {
                on_ok(paths[0]);
            }
        }, startingDir);
    }
} // namespace mui