#pragma once
#include <thread>
#include <vector>
#include <functional>
#include <string>
#include <imgui.h>
#include <filesystem>
#include <mutex>

// Forward declare SDL_GLContext to avoid including SDL.h in the header
using SDL_GLContext = struct SDL_GLContextState*;
struct SDL_Renderer;
struct SDL_Texture;
struct ImGuiStyle; // Forward declare ImGuiStyle

#include "../dialogs/dialogs.hpp"
#include "shadows.hpp"

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

    public:
        App() = delete;
        ~App() = delete;
        App(const App&) = delete;
        App& operator=(const App&) = delete;
        App(App&&) = delete;
        App& operator=(App&&) = delete;

    private:
        // --- ADD THESE SHADOW MEMBERS ---
        static SDL_Texture *s_raw_sdl_shadow_tex;
        static ImTextureID s_shadow_tex_id;
        static ImVec4 s_shadow_uvs[10];

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
        static bool s_applyGrayscale;
        static bool s_applyComplementary;
        static bool s_applySepia;
        static bool s_applyInvert;
        static bool useTomlTheme;
        static std::function<void()> mainLoopCallback;
        static void processMessageBoxes();
        static SDL_GLContext glContext;
        static std::string filepath;

        // Thread-safe main queue components
        static std::vector<std::function<void()>> m_mainQueue;
        static std::mutex m_mainQueueMutex;
        static void drainMainQueue();
        static void InitShadows();
        static void ShutdownShadows();

    public:
        static ImTextureID GetShadowTexture();
        static const ImVec4 *GetShadowUVs();
        static ImVec4 GetThemeShadowColor();

        static void setMainLoopCallback(std::function<void()> cb);
        static void setLayoutBuilder(std::function<void(DockBuilder &)> cb);
        static void init(const std::string &title="MUI", int width=1280, int height=720, bool useOpenGL = true, bool enableShadows = true);
        static void run();
        static void quit();
        static void shutdown();
        static void assertMainThread();
        static void queueMain(std::function<void()> callback);
        static void setTheme(ThemeType type);
        static void setTheme(const std::string &filepath, const std::string &themeName);
        static void setTheme(const std::string &themeName);
        static std::vector<std::string> getAvailableThemes();
        static ThemeType getTheme() { return currentTheme; }
        static void addMessageBox(ActiveMessageBox &&mb);
        static SDL_GLContext getGLContext();
        static const std::string& getFilepath() { return filepath; }
        static float getDpiScale() { return currentDpiScale; }
        static void requestDpiUpdate() { dpiNeedsUpdate = true; }
        static bool isUsingTomlTheme() { return useTomlTheme; }
        static const std::string& getCurrentThemeName() { return currentThemeName; }
        static const std::string& getCurrentThemeFile() { return currentThemeFile; }

        // Color modifier methods
        static void setApplyGrayscale(bool apply);
        static bool getApplyGrayscale();
        static void setApplyComplementary(bool apply);
        static bool getApplyComplementary();
        static void setApplySepia(bool apply);
        static bool getApplySepia();
        static void setApplyInvert(bool apply);
        static bool getApplyInvert();
        static void resetColorModifiers();
    };

} // namespace mui