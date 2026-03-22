#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <imgui.h>
#include <filesystem>

// Forward declare SDL_GLContext to avoid including SDL.h in the header
typedef struct SDL_GLContextState *SDL_GLContext;
struct SDL_Renderer;

#include "../dialogs/dialogs.hpp"

namespace mui
{
    class Window;
    class DockBuilder;
    using Identifier = ImGuiID;
    extern bool g_use_opengl;
    extern SDL_Renderer *g_renderer;

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
        static std::vector<ActiveMessageBox> activeMessageBoxes;
        static std::function<void(DockBuilder &)> layoutBuilderCb;
        static bool layoutNeedsInit;
        static float currentDpiScale;
        static bool dpiNeedsUpdate;
        static ThemeType currentTheme;
        static std::string currentThemeFile;
        static std::string currentThemeName;
        static bool useTomlTheme;
        static std::function<void()> mainLoopCallback; 
        static void processMessageBoxes();
        static SDL_GLContext glContext;
        static std::string filepath;

    public:

        static void setMainLoopCallback(std::function<void()> cb);
        static void setLayoutBuilder(std::function<void(DockBuilder &)> cb);
        static void init(bool useOpenGL = false);
        static void run();
        static void quit();
        static void shutdown();
        static void assertMainThread();
        static void queueMain(std::function<void()> callback);
        static void setTheme(ThemeType type);
        static void setTheme(const std::string& filepath, const std::string& themeName);
        static void setTheme(const std::string& themeName);
        static std::vector<std::string> getAvailableThemes();
        static ThemeType getTheme() { return currentTheme; }
        static void addMessageBox(ActiveMessageBox &&mb);
        static SDL_GLContext getGLContext();
        static std::string getFilepath() { return filepath; }
        static float getDpiScale() { return currentDpiScale; }
        static void requestDpiUpdate() { dpiNeedsUpdate = true; }
        static bool isUsingTomlTheme() { return useTomlTheme; }
        static std::string getCurrentThemeName() { return currentThemeName; }
        static std::string getCurrentThemeFile() { return currentThemeFile; }
    };
} // namespace mui