#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <imgui.h>
#include <filesystem>

namespace mui
{
    class Window;
    using Identifier = ImGuiID;

    struct ActiveDialog
    {
        std::string key;
        std::function<void(const std::vector<std::filesystem::path> &)> on_ok;
        std::function<void()> on_cancel;
    };

    struct ActiveMessageBox
    {
        std::string title;
        std::string message;
        bool open_popup = true;
    };

    enum class ThemeType
    {
        Light,
        Dark
    };

    class App
    {
        friend class Window;

    private:
        static std::thread::id mainThreadId;
        static std::vector<Window *> activeWindows;
        static std::vector<ActiveDialog> activeDialogs;
        static std::vector<ActiveMessageBox> activeMessageBoxes;
        static std::function<void(ImGuiID)> layoutBuilderCb;
        static bool layoutNeedsInit;
        static float currentDpiScale;
        static bool dpiNeedsUpdate;
        static ThemeType currentTheme;
        static void processDialogs();
        static void processMessageBoxes();

    public:
        static void setLayoutBuilder(std::function<void(Identifier)> cb);
        static void init();
        static void run();
        static void quit();
        static void assertMainThread();
        static void queueMain(std::function<void()> callback);
        static void setTheme(ThemeType type);
        static ThemeType getTheme() { return currentTheme; }
        static void addDialog(ActiveDialog &&dialog);
        static void addMessageBox(ActiveMessageBox &&mb);
    };
} // namespace mui