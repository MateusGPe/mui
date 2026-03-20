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
namespace mui
{
    class Window;
    class DockBuilder;
    using Identifier = ImGuiID;
    extern bool g_use_opengl;
    extern SDL_Renderer *g_renderer;

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
        static std::vector<ActiveMessageBox> activeMessageBoxes;
        static std::function<void(DockBuilder &)> layoutBuilderCb;
        static bool layoutNeedsInit;
        static float currentDpiScale;
        static bool dpiNeedsUpdate;
        static ThemeType currentTheme;
        static std::function<void()> mainLoopCallback; 
        static void processMessageBoxes();
        static SDL_GLContext glContext;

    public:
        static void setMainLoopCallback(std::function<void()> cb);
        static void setLayoutBuilder(std::function<void(DockBuilder &)> cb);
        static void init(bool useOpenGL = false);
        static void run();
        static void quit();
        static void assertMainThread();
        static void queueMain(std::function<void()> callback);
        static void setTheme(ThemeType type);
        static ThemeType getTheme() { return currentTheme; }
        static void addMessageBox(ActiveMessageBox &&mb);
        static SDL_GLContext getGLContext();
    };
} // namespace mui