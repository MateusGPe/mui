#include "dialogs.hpp"
#include "ifd/ImFileDialog.h"
#include "../core/app.hpp"
#include <vector>

namespace mui
{
    namespace
    {
        struct DialogState
        {
            std::string key;
            std::function<void(const std::string &)> on_ok_single;
            std::function<void(const std::vector<std::string> &)> on_ok_multi;
            std::function<void()> on_cancel;
        };

        static DialogState g_current_dialog_state;
    } // namespace

    void Dialogs::processDialogs()
    {
        if (g_current_dialog_state.key.empty())
        {
            return;
        }

        auto &instance = ifd::FileDialog::Instance();
        if (instance.IsDone(g_current_dialog_state.key))
        {
            if (instance.HasResult())
            {
                if (g_current_dialog_state.on_ok_multi)
                {
                    std::vector<std::string> paths;
                    for (const auto &p : instance.GetResults())
                    {
                        paths.push_back(p.u8string());
                    }
                    g_current_dialog_state.on_ok_multi(paths);
                }
                else if (g_current_dialog_state.on_ok_single)
                {
                    g_current_dialog_state.on_ok_single(instance.GetResult().u8string());
                }
            }
            else
            {
                if (g_current_dialog_state.on_cancel)
                {
                    g_current_dialog_state.on_cancel();
                }
            }
            instance.Close();
            g_current_dialog_state = {}; // Clear state
        }
    }

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
        if (!g_current_dialog_state.key.empty())
        {
            // Another dialog is already open.
            return;
        }
        std::string key = "openfile##" + title;

        if (ifd::FileDialog::Instance().Open(key, title, filter, false, startingDir))
        {
            g_current_dialog_state = {key, on_ok, nullptr, on_cancel};
        }
    }

    void Dialogs::openFiles(const std::string &title, const std::string &filter, std::function<void(const std::vector<std::string> &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir)
    {
        if (!g_current_dialog_state.key.empty())
        {
            return;
        }
        std::string key = "openfiles##" + title;

        if (ifd::FileDialog::Instance().Open(key, title, filter, true, startingDir))
        {
            g_current_dialog_state = {key, nullptr, on_ok, on_cancel};
        }
    }

    void Dialogs::saveFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir)
    {
        if (!g_current_dialog_state.key.empty())
        {
            return;
        }
        std::string key = "savefile##" + title;

        if (ifd::FileDialog::Instance().Save(key, title, filter, startingDir))
        {
            g_current_dialog_state = {key, on_ok, nullptr, on_cancel};
        }
    }
} // namespace mui