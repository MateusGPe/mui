#pragma once
#include <string>
#include <functional>
#include <vector>

namespace mui
{
    enum class MessageBoxType
    {
        Info,
        Warning,
        Error,
        Question
    };

    struct MessageBoxButton
    {
        std::string text;
        std::function<void()> callback;
    };

    struct ActiveMessageBox
    {
        std::string title;
        std::string message;
        MessageBoxType type = MessageBoxType::Info;
        std::vector<MessageBoxButton> buttons;
        bool open_popup = true;

        bool is_input = false;
        std::string input_buffer;
        std::function<void(const std::string &)> on_input_ok;
        std::string popup_id;
    };

    class Dialogs
    {
    public:
        // Simple message boxes
        static void msgBox(const std::string &title, const std::string &message); // Alias for Info
        static void msgBoxInfo(const std::string &title, const std::string &message);
        static void msgBoxWarning(const std::string &title, const std::string &message);
        static void msgBoxError(const std::string &title, const std::string &message);
        static void msgBoxConfirm(const std::string &title, const std::string &message, std::function<void()> on_ok, std::function<void()> on_cancel = nullptr);
        static void msgBoxQuestion(const std::string &title, const std::string &message, std::function<void()> on_yes, std::function<void()> on_no = nullptr);
        static void msgBoxCustom(const std::string &title, const std::string &message, MessageBoxType type, std::vector<MessageBoxButton> &&buttons);
        static void msgBoxInput(const std::string &title, const std::string &message, const std::string &default_text, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel = nullptr);

        // File dialogs using mui::FileDialog
        static void openFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel = nullptr, const std::string &startingDir = "");
        static void openFiles(const std::string &title, const std::string &filter, std::function<void(const std::vector<std::string> &)> on_ok, std::function<void()> on_cancel = nullptr, const std::string &startingDir = "");
        static void saveFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel = nullptr, const std::string &startingDir = "");
        static void selectFolder(const std::string &title, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel = nullptr, const std::string &startingDir = "");

        // To be called from the main loop to process dialog results.
        static void processDialogs();

        // To be called from App::processMessageBoxes to render a message box.
        static void renderMessageBox(ActiveMessageBox &mb, bool &isOpen);
    };
} // namespace mui