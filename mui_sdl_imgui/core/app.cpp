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
#include "../dialogs/ImFileDialog.h"

namespace mui
{
    std::thread::id App::mainThreadId;

    static SDL_Window *g_window = nullptr;
    static SDL_Renderer *g_renderer = nullptr;
    static bool g_running = false;
    std::vector<Window *> App::activeWindows;
    std::vector<ActiveDialog> App::activeDialogs;
    std::vector<ActiveMessageBox> App::activeMessageBoxes;

    std::function<void(ImGuiID)> App::layoutBuilderCb = nullptr;
    bool App::layoutNeedsInit = true;

    // Initialize DPI variables
    float App::currentDpiScale = 1.0f;
    bool App::dpiNeedsUpdate = false;
    ThemeType App::currentTheme = ThemeType::Light;

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
        
        // Apply initial theme and load fonts directly for the first frame
        if (currentTheme == ThemeType::Dark) mui::Theme::applyDarkStyle(currentDpiScale);
        else mui::Theme::applyStyle(currentDpiScale);
        mui::Theme::loadSystemFont(16.0f * currentDpiScale); // Default font size

        ImGui_ImplSDL3_InitForSDLRenderer(g_window, g_renderer);
        ImGui_ImplSDLRenderer3_Init(g_renderer);

        // Initialize ImFileDialog
        ifd::FileDialog::Instance().CreateTexture = [](uint8_t *data, int w, int h, char fmt) -> void *
        {
            if (!g_renderer)
                return nullptr;
            SDL_Texture *texture = SDL_CreateTexture(g_renderer, fmt == 0 ? SDL_PIXELFORMAT_BGRA8888 : SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h);
            if (!texture)
                return nullptr;
            SDL_UpdateTexture(texture, nullptr, data, w * 4);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            return (void *)texture;
        };
        ifd::FileDialog::Instance().DeleteTexture = [](void *texture)
        {
            if (texture)
                SDL_DestroyTexture(static_cast<SDL_Texture *>(texture));
        };
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

            // --- REBUILD FONTS & STYLE IF DPI CHANGED OR THEME SWITCHED ---
            if (dpiNeedsUpdate) {
                ImGui_ImplSDLRenderer3_DestroyDeviceObjects();
                io.Fonts->Clear();

                // Reset ImGui style to defaults before reapplying multipliers
                ImGui::GetStyle() = ImGuiStyle();

                // Apply the current theme
                if (currentTheme == ThemeType::Dark) mui::Theme::applyDarkStyle(currentDpiScale);
                else mui::Theme::applyStyle(currentDpiScale);
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

            processDialogs();
            processMessageBoxes();

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

    void App::addDialog(ActiveDialog &&dialog)
    {
        activeDialogs.push_back(std::move(dialog));
    }

    void App::addMessageBox(ActiveMessageBox &&mb)
    {
        activeMessageBoxes.push_back(std::move(mb));
    }

    void App::processDialogs()
    {
        for (auto it = activeDialogs.begin(); it != activeDialogs.end();)
        {
            if (ifd::FileDialog::Instance().IsDone(it->key))
            {
                if (ifd::FileDialog::Instance().HasResult())
                {
                    if (it->on_ok)
                    {
                        it->on_ok(ifd::FileDialog::Instance().GetResults());
                    }
                }
                else
                {
                    if (it->on_cancel)
                    {
                        it->on_cancel();
                    }
                }
                ifd::FileDialog::Instance().Close();
                it = activeDialogs.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void App::processMessageBoxes()
    {
        for (auto it = activeMessageBoxes.begin(); it != activeMessageBoxes.end();)
        {
            std::string popup_id = it->title + "##msgbox";
            if (it->open_popup)
            {
                ImGui::OpenPopup(popup_id.c_str());
                it->open_popup = false;
            }

            bool isOpen = true;
            if (ImGui::BeginPopupModal(popup_id.c_str(), &isOpen))
            {
                ImGui::TextWrapped("%s", it->message.c_str());
                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                    isOpen = false;
                }
                ImGui::EndPopup();
            }

            if (!isOpen)
            {
                it = activeMessageBoxes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void App::setTheme(ThemeType type)
    {
        if (currentTheme == type) return; // No change needed
        currentTheme = type;
        dpiNeedsUpdate = true; // Signal the main loop to reapply theme and rebuild fonts
    }
} // namespace mui
