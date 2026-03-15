#pragma once
#include <functional>
#include <thread>
#include <vector>
#include <string>

namespace mui
{
    using Identifier = unsigned int;

    enum class ThemeType
    {
        Light,
        Dark
    };
    class Window;
    class App
    {
    public:
        static std::thread::id mainThreadId;
        static std::vector<Window *> activeWindows;

        static void init();
        static void run();
        static void quit();
        static void assertMainThread();
        static void queueMain(std::function<void()> callback);
        static void setLayoutBuilder(std::function<void(Identifier)> cb);
        static void setTheme(ThemeType type);

    private:
        static std::function<void(Identifier)> layoutBuilderCb;
        static bool layoutNeedsInit;
        static float currentDpiScale;
        static bool dpiNeedsUpdate;
        static ThemeType currentTheme;
    };
} // namespace mui
