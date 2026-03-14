#include "app.hpp"
#include "window.hpp"
#include "theme.hpp"
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <stdexcept>
#include <algorithm>

namespace mui
{
    std::thread::id App::mainThreadId;

    static SDL_Window *g_window = nullptr;
    static SDL_Renderer *g_renderer = nullptr;
    static bool g_running = false;
    std::vector<Window *> App::activeWindows;

    std::function<void(ImGuiID)> App::layoutBuilderCb = nullptr;
    bool App::layoutNeedsInit = true;

    // Initialize DPI variables
    float App::currentDpiScale = 1.0f;
    bool App::dpiNeedsUpdate = false;

    void App::setLayoutBuilder(std::function<void(Identifier)> cb)
    {
        layoutBuilderCb = std::move(cb);
    }

    void App::init()
    {
        mainThreadId = std::this_thread::get_id();

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
        }

        Uint32 window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        g_window = SDL_CreateWindow("MUI Master", 1280, 720, window_flags);
        if (!g_window)
        {
            throw std::runtime_error("SDL_CreateWindow Error");
        }

        g_renderer = SDL_CreateRenderer(g_window, nullptr);
        if (!g_renderer)
        {
            throw std::runtime_error("SDL_CreateRenderer Error");
        }

        // Fetch initial DPI
        currentDpiScale = SDL_GetWindowDisplayScale(g_window);
        if (currentDpiScale <= 0.0f)
            currentDpiScale = 1.0f;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        mui::Theme::applyStyle(currentDpiScale);
        mui::Theme::loadSystemFont(18.0f * currentDpiScale);

        ImGui_ImplSDL3_InitForSDLRenderer(g_window, g_renderer);
        ImGui_ImplSDLRenderer3_Init(g_renderer);
    }

    void App::run()
    {
        SDL_ShowWindow(g_window);

        g_running = true;
        ImGuiIO &io = ImGui::GetIO();

        while (g_running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT)
                {
                    g_running = false;
                }
                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(g_window))
                {
                    g_running = false;
                }

                // --- DETECT DPI CHANGE ---
                if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED && event.window.windowID == SDL_GetWindowID(g_window))
                {
                    currentDpiScale = SDL_GetWindowDisplayScale(g_window);
                    if (currentDpiScale <= 0.0f)
                        currentDpiScale = 1.0f;
                    dpiNeedsUpdate = true;
                }
            }

            // --- REBUILD FONTS & STYLE BEFORE NEW FRAME ---
            if (dpiNeedsUpdate)
            {
                ImGui_ImplSDLRenderer3_DestroyDeviceObjects();
                io.Fonts->Clear();

                // Reset ImGui style to defaults before reapplying multipliers
                ImGui::GetStyle() = ImGuiStyle();

                mui::Theme::applyStyle(currentDpiScale);
                mui::Theme::loadSystemFont(18.0f * currentDpiScale);

                io.Fonts->Build();

                ImGui_ImplSDLRenderer3_CreateDeviceObjects();

                dpiNeedsUpdate = false;
            }

            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

            if (layoutNeedsInit && layoutBuilderCb)
            {
                ImGui::DockBuilderRemoveNode(dockspace_id);
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

                layoutBuilderCb(dockspace_id);

                ImGui::DockBuilderFinish(dockspace_id);
                layoutNeedsInit = false;
            }

            ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

            for (auto *win : activeWindows)
            {
                win->render();
            }

            activeWindows.erase(std::remove_if(activeWindows.begin(), activeWindows.end(),
                                               [](const auto *win)
                                               { return !win->isWindowOpen(); }),
                                activeWindows.end());

            ImGui::Render();
            SDL_SetRenderDrawColor(g_renderer, 45, 45, 45, 255);
            SDL_RenderClear(g_renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_renderer);
            SDL_RenderPresent(g_renderer);

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }
    }

    void App::quit() { g_running = false; }

    void App::assertMainThread()
    {
        if (std::this_thread::get_id() != mainThreadId)
        {
            throw std::runtime_error("MUI Error: UI interaction outside of the main thread.");
        }
    }

    void App::queueMain(std::function<void()> callback)
    {
        callback();
    }
} // namespace mui
