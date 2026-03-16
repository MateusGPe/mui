#pragma once
#include <string>
#include <functional>
#include <vector>

namespace mui
{
    class Dialogs
    {
    public:
        // Simple message boxes
        static void msgBox(const std::string &title, const std::string &message);
        static void msgBoxError(const std::string &title, const std::string &message);

        // File dialogs using mui::FileDialog
        static void openFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel = nullptr, const std::string &startingDir = "");
        static void openFiles(const std::string &title, const std::string &filter, std::function<void(const std::vector<std::string> &)> on_ok, std::function<void()> on_cancel = nullptr, const std::string &startingDir = "");
        static void saveFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel = nullptr, const std::string &startingDir = "");
    };
} // namespace mui