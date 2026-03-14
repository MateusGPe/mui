#include "app.hpp"
#include "window.hpp"
#include <SDL3/SDL.h>
#include <imgui.h>
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

    void App::init()
    {
        mainThreadId = std::this_thread::get_id();

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
        }

        Uint32 window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;
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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplSDL3_InitForSDLRenderer(g_window, g_renderer);
        ImGui_ImplSDLRenderer3_Init(g_renderer);
    }

    void App::run()
    {
        g_running = true;
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
            }

            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            for (auto *win : activeWindows)
            {
                win->render();
            }

            // Remove windows that have been closed. This uses the remove-erase idiom.
            activeWindows.erase(std::remove_if(activeWindows.begin(), activeWindows.end(),
                                               [](const auto *win) { return !win->isWindowOpen(); }),
                              activeWindows.end());

            ImGui::Render();
            SDL_SetRenderDrawColor(g_renderer, 45, 45, 45, 255);
            SDL_RenderClear(g_renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_renderer);
            SDL_RenderPresent(g_renderer);
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
