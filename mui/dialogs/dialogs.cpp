#include "dialogs.hpp"
#include "../core/app.hpp"
#include "../core/scoped.hpp"
#include "IconsFontAwesome6.h"
#include "file/mui_dialog.h"
#include <cstdlib> // for getenv
#include <imgui.h>
#include <imgui_internal.h>
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

    std::string get_effective_starting_dir(const std::string &startingDir)
    {
      if (!startingDir.empty())
      {
        return startingDir;
      }

      try
      {
        // This can throw if the current working directory has been deleted
        return std::filesystem::current_path().string();
      }
      catch (const std::filesystem::filesystem_error &)
      {
// Fallback to a safe directory if current_path fails.
#ifdef _WIN32
        const char *user_profile = getenv("USERPROFILE");
        if (user_profile)
        {
          return std::string(user_profile);
        }
        const char *home_drive = getenv("HOMEDRIVE");
        const char *home_path = getenv("HOMEPATH");
        if (home_drive && home_path)
        {
          return std::string(home_drive) + std::string(home_path);
        }
        return "C:\\"; // Absolute last resort for Windows
#else
        const char *home = getenv("HOME");
        if (home)
        {
          return std::string(home);
        }
        return "/"; // Absolute last resort for POSIX
#endif
      }
    }
  } // namespace

  void Dialogs::processDialogs()
  {
    if (g_current_dialog_state.key.empty())
    {
      return;
    }

    auto &instance = mui_dlg::FileDialog::Instance();
    if (instance.IsDone(g_current_dialog_state.key))
    {
      if (instance.HasResult())
      {
        if (g_current_dialog_state.on_ok_multi)
        {
          std::vector<std::string> paths;
          for (const auto &p : instance.GetResults())
          {
            paths.push_back(p.string());
          }
          g_current_dialog_state.on_ok_multi(paths);
        }
        else if (g_current_dialog_state.on_ok_single)
        {
          g_current_dialog_state.on_ok_single(instance.GetResult().string());
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
    App::addMessageBox({title, message, MessageBoxType::Info, {{"OK", nullptr}}});
  }

  void Dialogs::msgBoxInfo(const std::string &title, const std::string &message)
  {
    App::addMessageBox({title, message, MessageBoxType::Info, {{"OK", nullptr}}});
  }

  void Dialogs::msgBoxWarning(const std::string &title,
                              const std::string &message)
  {
    App::addMessageBox(
        {title, message, MessageBoxType::Warning, {{"OK", nullptr}}});
  }

  void Dialogs::msgBoxError(const std::string &title,
                            const std::string &message)
  {
    App::addMessageBox(
        {title, message, MessageBoxType::Error, {{"OK", nullptr}}});
  }

  void Dialogs::msgBoxConfirm(const std::string &title,
                              const std::string &message,
                              std::function<void()> on_ok,
                              std::function<void()> on_cancel)
  {
    App::addMessageBox({title,
                        message,
                        MessageBoxType::Question,
                        {{"OK", on_ok}, {"Cancel", on_cancel}}});
  }

  void Dialogs::msgBoxQuestion(const std::string &title,
                               const std::string &message,
                               std::function<void()> on_yes,
                               std::function<void()> on_no)
  {
    App::addMessageBox({title,
                        message,
                        MessageBoxType::Question,
                        {{"Yes", on_yes}, {"No", on_no}}});
  }

  void Dialogs::msgBoxCustom(const std::string &title, const std::string &message,
                             MessageBoxType type,
                             std::vector<MessageBoxButton> &&buttons)
  {
    App::addMessageBox({title, message, type, std::move(buttons)});
  }

  void Dialogs::openFile(const std::string &title, const std::string &filter,
                         std::function<void(const std::string &)> on_ok,
                         std::function<void()> on_cancel,
                         const std::string &startingDir)
  {
    if (!g_current_dialog_state.key.empty())
    {
      // Another dialog is already open.
      return;
    }
    std::string key = "openfile##" + title;

    std::string effectiveDir = get_effective_starting_dir(startingDir);
    if (mui_dlg::FileDialog::Instance().Open(key, title, filter, false,
                                             effectiveDir))
    {
      g_current_dialog_state = {key, on_ok, nullptr, on_cancel};
    }
  }

  void Dialogs::openFiles(
      const std::string &title, const std::string &filter,
      std::function<void(const std::vector<std::string> &)> on_ok,
      std::function<void()> on_cancel, const std::string &startingDir)
  {
    if (!g_current_dialog_state.key.empty())
    {
      return;
    }
    std::string key = "openfiles##" + title;

    std::string effectiveDir = get_effective_starting_dir(startingDir);
    if (mui_dlg::FileDialog::Instance().Open(key, title, filter, true,
                                             effectiveDir))
    {
      g_current_dialog_state = {key, nullptr, on_ok, on_cancel};
    }
  }

  void Dialogs::saveFile(const std::string &title, const std::string &filter,
                         std::function<void(const std::string &)> on_ok,
                         std::function<void()> on_cancel,
                         const std::string &startingDir)
  {
    if (!g_current_dialog_state.key.empty())
    {
      return;
    }
    std::string key = "savefile##" + title;

    std::string effectiveDir = get_effective_starting_dir(startingDir);
    if (mui_dlg::FileDialog::Instance().Save(key, title, filter, effectiveDir))
    {
      g_current_dialog_state = {key, on_ok, nullptr, on_cancel};
    }
  }

  void Dialogs::renderMessageBox(ActiveMessageBox &mb, bool &isOpen)
  {
    // --- Icon and Color based on type ---
    const char *icon = ICON_FA_CIRCLE_INFO;
    ImVec4 iconColor =
        ImVec4(0.12f, 0.53f, 0.90f, 1.00f); // Blue for Info from light theme

    // Use theme-aware colors if possible
    if (App::getTheme() == ThemeType::Dark)
    {
      iconColor = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // Blue from dark theme
    }

    switch (mb.type)
    {
    case MessageBoxType::Warning:
      icon = ICON_FA_TRIANGLE_EXCLAMATION;
      iconColor = ImVec4(1.0f, 0.75f, 0.0f, 1.0f); // Amber/Yellow for both themes
      break;
    case MessageBoxType::Error:
      icon = ICON_FA_CIRCLE_XMARK;
      iconColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // Red for both themes
      break;
    case MessageBoxType::Question:
      icon = ICON_FA_CIRCLE_QUESTION;
      // Use info color, which is already the default.
      break;
    case MessageBoxType::Info:
    default:
      // Defaults are already set
      break;
    }

    // --- Layout for Icon and Text ---
    if (ImGui::BeginTable("msgbox_layout", 2, ImGuiTableFlags_SizingFixedFit))
    {
      ImGui::TableSetupColumn("icon", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("text", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      {
        ScopedColor color(ImGuiCol_Text, iconColor);
        ImGui::SetWindowFontScale(2.0f);
        ImGui::TextUnformatted(icon);
        ImGui::SetWindowFontScale(1.0f);
      }

      ImGui::TableSetColumnIndex(1);
      ImGui::TextWrapped("%s", mb.message.c_str());

      ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- Buttons ---
    const float button_width = 100.0f;
    const float button_spacing = ImGui::GetStyle().ItemSpacing.x;
    float total_buttons_width = 0.0f;
    if (!mb.buttons.empty())
    {
      total_buttons_width = (mb.buttons.size() * button_width) +
                            ((mb.buttons.size() - 1) * button_spacing);
    }

    // Right-align the button group
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         ImGui::GetContentRegionAvail().x - total_buttons_width);

    for (size_t i = 0; i < mb.buttons.size(); ++i)
    {
      const auto &button = mb.buttons[i];
      if (i > 0)
      {
        ImGui::SameLine();
      }
      ScopedID btnId(static_cast<int>(i));
      if (ImGui::Button(button.text.c_str(), ImVec2(button_width, 0)))
      {
        if (button.callback)
        {
          button.callback();
        }
        ImGui::CloseCurrentPopup();
        isOpen = false;
      }
    }
  }
} // namespace mui