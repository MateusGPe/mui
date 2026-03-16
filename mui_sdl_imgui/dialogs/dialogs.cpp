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
        
        // Changed to mui::FileDialog
        mui::FileDialog::Instance().Open(key, title, filter, false, startingDir);

        App::addDialog({key,
                        [on_ok](const std::vector<std::filesystem::path> &paths)
                        {
                            if (on_ok && !paths.empty())
                            {
                                on_ok(paths[0].u8string());
                            }
                        },
                        on_cancel});
    }

    void Dialogs::openFiles(const std::string &title, const std::string &filter, std::function<void(const std::vector<std::string> &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir)
    {
        std::string key = "openfiles##" + title;
        
        // Changed to mui::FileDialog
        mui::FileDialog::Instance().Open(key, title, filter, true, startingDir);

        App::addDialog({key,
                        [on_ok](const std::vector<std::filesystem::path> &paths)
                        {
                            if (on_ok && !paths.empty())
                            {
                                std::vector<std::string> result_paths;
                                for (const auto &p : paths)
                                {
                                    result_paths.push_back(p.u8string());
                                }
                                on_ok(result_paths);
                            }
                        },
                        on_cancel});
    }

    void Dialogs::saveFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir)
    {
        std::string key = "savefile##" + title;
        
        // Changed to mui::FileDialog
        mui::FileDialog::Instance().Save(key, title, filter, startingDir);

        App::addDialog({key,
                        [on_ok](const std::vector<std::filesystem::path> &paths)
                        {
                            if (on_ok && !paths.empty())
                            {
                                on_ok(paths[0].u8string());
                            }
                        },
                        on_cancel});
    }
} // namespace mui