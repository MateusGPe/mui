#include "app.hpp"
#include "../dialogs/dialogs.hpp"
#include "../dialogs/file/mui_dialog.h"
#include "../include/IconsFontAwesome6.h"
#include "docking.hpp"
#include "theme.hpp"
#include "window.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <stdexcept>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

namespace mui
{
  std::thread::id App::mainThreadId;
  std::string App::filepath = "";

  static SDL_Window *g_window = nullptr;
  SDL_Renderer *g_renderer = nullptr;
  SDL_GLContext App::glContext = nullptr;
  bool g_use_opengl = false;
  bool g_running = false;
  std::vector<Window *> App::activeWindows;
  std::vector<ActiveMessageBox> App::activeMessageBoxes;

  std::function<void(DockBuilder &)> App::layoutBuilderCb = nullptr;
  bool App::layoutNeedsInit = true;

  // Initialize DPI variables
  float App::currentDpiScale = 1.0f;
  bool App::dpiNeedsUpdate = false;
  ThemeType App::currentTheme = ThemeType::Dark;
  std::string App::currentThemeFile = "";
  std::string App::currentThemeName = "";
  bool App::s_applyGrayscale = false;
  bool App::s_applyComplementary = false;
  bool App::s_applySepia = false;
  bool App::s_applyInvert = false;
  bool App::useTomlTheme = false;
  // Define the static shadow variables
  SDL_Texture *App::s_raw_sdl_shadow_tex = nullptr;
  ImTextureID App::s_shadow_tex_id = (ImTextureID)(intptr_t)nullptr;
  ImVec4 App::s_shadow_uvs[10] = {};

  ImTextureID App::GetShadowTexture() { return s_shadow_tex_id; }
  const ImVec4 *App::GetShadowUVs() { return s_shadow_uvs; }

  ImVec4 App::GetThemeShadowColor()
  {
    // Use the WindowBg color to determine if we are currently in a Light or Dark
    // theme
    ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    float luminance = bg.x * 0.299f + bg.y * 0.587f + bg.z * 0.114f;

    if (luminance > 0.5f)
    {
      return ImVec4(0.0f, 0.0f, 0.0f, 0.12f); // Softer shadow for light themes
    }
    else
    {
      return ImVec4(0.0f, 0.0f, 0.0f,
                    0.45f); // Darker, stronger shadow for dark themes
    }
  }

  void App::InitShadows()
  {
    if (s_shadow_tex_id != (ImTextureID)(intptr_t)nullptr)
      return; // Already initialized

    ImGuiShadows::ShadowTextureConfig cfg;
    ImGuiShadows::ShadowTextureData data =
        ImGuiShadows::GenerateShadowTexture(cfg);

    if (g_use_opengl)
    {
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.Width, data.Height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, data.PixelsRGBA32.data());
      glBindTexture(GL_TEXTURE_2D, 0);
      s_shadow_tex_id = (ImTextureID)(intptr_t)texture;
    }
    else
    {
      s_raw_sdl_shadow_tex =
          SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA32,
                            SDL_TEXTUREACCESS_STATIC, data.Width, data.Height);
      SDL_UpdateTexture(s_raw_sdl_shadow_tex, nullptr, data.PixelsRGBA32.data(),
                        data.Width * 4);
      s_shadow_tex_id = (ImTextureID)(intptr_t)s_raw_sdl_shadow_tex;
    }

    // Copy UV coordinates
    for (int i = 0; i < 10; ++i)
    {
      s_shadow_uvs[i] = data.Uvs[i];
    }
  }

  void App::ShutdownShadows()
  {
    if (s_shadow_tex_id != (ImTextureID)(intptr_t)nullptr)
    {
      if (g_use_opengl)
      {
        GLuint texID = (GLuint)(intptr_t)s_shadow_tex_id;
        glDeleteTextures(1, &texID);
      }
      else if (s_raw_sdl_shadow_tex)
      {
        SDL_DestroyTexture(s_raw_sdl_shadow_tex);
        s_raw_sdl_shadow_tex = nullptr;
      }
      s_shadow_tex_id = (ImTextureID)(intptr_t)nullptr;
    }
  }
  // Initialize thread-safe queue components
  std::vector<std::function<void()>> App::m_mainQueue;
  std::mutex App::m_mainQueueMutex;

  void App::setLayoutBuilder(std::function<void(DockBuilder &)> cb)
  {
    layoutBuilderCb = std::move(cb);
  }

  void App::init(bool useOpenGL, bool enableShadows)
  {
    const char *base_path = SDL_GetBasePath();
    if (base_path)
    {
      filepath = base_path;
      SDL_free((void *)base_path);
    }

    mainThreadId = std::this_thread::get_id();
    g_use_opengl = useOpenGL;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
      throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
    }

    Uint32 window_flags =
        SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (g_use_opengl)
    {
      window_flags |= SDL_WINDOW_OPENGL;

      // GL 3.3 Core
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                          SDL_GL_CONTEXT_PROFILE_CORE);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    }

    g_window = SDL_CreateWindow("MUI Master", 1280, 720, window_flags);
    if (!g_window)
    {
      throw std::runtime_error("SDL_CreateWindow Error");
    }

    if (g_use_opengl)
    {
      glContext = SDL_GL_CreateContext(g_window);
      if (!glContext)
      {
        throw std::runtime_error(std::string("SDL_GL_CreateContext Error: ") +
                                 SDL_GetError());
      }
      SDL_GL_MakeCurrent(g_window, glContext);
      SDL_GL_SetSwapInterval(1); // Enable vsync
    }
    else
    {
      g_renderer = SDL_CreateRenderer(g_window, nullptr);
      if (!g_renderer)
      {
        throw std::runtime_error("SDL_CreateRenderer Error");
      }
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
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Apply initial theme and load fonts directly for the first frame
    if (useTomlTheme)
    {
      if (!mui::Theme::loadThemeFromToml(currentThemeFile, currentThemeName,
                                         currentDpiScale))
      {
        // If TOML fails, fallback to dark and reset modifiers
        App::resetColorModifiers();
        mui::Theme::applyDarkStyle(currentDpiScale);
      }
    }
    else
    {
      if (currentTheme == ThemeType::Dark)
        mui::Theme::applyDarkStyle(currentDpiScale);
      else
        mui::Theme::applyStyle(currentDpiScale);
    }
    mui::Theme::loadSystemFont(16.0f * currentDpiScale); // Default font size

    if (g_use_opengl)
    {
      ImGui_ImplSDL3_InitForOpenGL(g_window, glContext);
      ImGui_ImplOpenGL3_Init("#version 330");
    }
    else
    {
      ImGui_ImplSDL3_InitForSDLRenderer(g_window, g_renderer);
      ImGui_ImplSDLRenderer3_Init(g_renderer);
    }

    // Initialize ImFileDialog
    mui_dlg::FileDialog::Instance().CreateTexture =
        [=](uint8_t *data, int w, int h, char fmt) -> void *
    {
      if (g_use_opengl)
      {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                     (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return (void *)(uintptr_t)texture;
      }
      else
      {
        SDL_Texture *texture = SDL_CreateTexture(
            g_renderer,
            (fmt == 0) ? SDL_PIXELFORMAT_BGRA32 : SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STATIC, w, h);
        if (texture)
        {
          SDL_UpdateTexture(texture, nullptr, data, w * 4);
        }
        return texture;
      }
    };

    mui_dlg::FileDialog::Instance().DeleteTexture = [=](void *texture)
    {
      if (g_use_opengl)
      {
        GLuint texID = (GLuint)(uintptr_t)texture;
        glDeleteTextures(1, &texID);
      }
      else
      {
        SDL_DestroyTexture((SDL_Texture *)texture);
      }
    };

    if (enableShadows)
    {
      InitShadows();
    }
  }

  std::function<void()> App::mainLoopCallback = nullptr;

  void App::setMainLoopCallback(std::function<void()> cb)
  {
    mainLoopCallback = std::move(cb);
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
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event.window.windowID == SDL_GetWindowID(g_window))
        {
          g_running = false;
        }

        // --- DETECT DPI CHANGE ---
        if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED &&
            event.window.windowID == SDL_GetWindowID(g_window))
        {
          currentDpiScale = SDL_GetWindowDisplayScale(g_window);
          if (currentDpiScale <= 0.0f)
            currentDpiScale = 1.0f;
          dpiNeedsUpdate = true;
        }
      }

      // Drain cross-thread UI tasks before starting the new ImGui frame
      drainMainQueue();

      // --- REBUILD FONTS & STYLE IF DPI CHANGED OR THEME SWITCHED ---
      if (dpiNeedsUpdate)
      {
        if (g_use_opengl)
        {
          ImGui_ImplOpenGL3_DestroyDeviceObjects();
        }
        else
        {
          ImGui_ImplSDLRenderer3_DestroyDeviceObjects();
        }
        io.Fonts->Clear();

        // Reset ImGui style to defaults before reapplying multipliers
        ImGui::GetStyle() = ImGuiStyle();

        // Apply the current theme
        if (useTomlTheme)
        {
          if (!mui::Theme::loadThemeFromToml(currentThemeFile, currentThemeName,
                                             currentDpiScale))
          {
            // If TOML fails, fallback to dark and reset modifiers
            resetColorModifiers();
            mui::Theme::applyDarkStyle(currentDpiScale);
          }
        }
        else
        {
          if (currentTheme == ThemeType::Dark)
            mui::Theme::applyDarkStyle(currentDpiScale);
          else
            mui::Theme::applyStyle(currentDpiScale);
        }

        // Apply color modifiers if enabled
        if (s_applyGrayscale)
          Theme::applyGrayscale();
        if (s_applySepia)
          Theme::applySepia();
        if (s_applyInvert)
          Theme::applyInvert();
        if (s_applyComplementary)
          Theme::applyComplementary();

        mui::Theme::loadSystemFont(18.0f * currentDpiScale);

        io.Fonts->Build();

        if (g_use_opengl)
        {
          ImGui_ImplOpenGL3_CreateDeviceObjects();
        }
        else
        {
          ImGui_ImplSDLRenderer3_CreateDeviceObjects();
        }

        dpiNeedsUpdate = false;
      }

      if (g_use_opengl)
      {
        ImGui_ImplOpenGL3_NewFrame();
      }
      else
      {
        ImGui_ImplSDLRenderer3_NewFrame();
      }
      ImGui_ImplSDL3_NewFrame();
      ImGui::NewFrame();

      ImGuiViewport *viewport = ImGui::GetMainViewport();
      ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

      if (layoutNeedsInit && layoutBuilderCb)
      {
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
          ImGui::DockBuilderRemoveNode(dockspace_id);
          ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
          ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

          DockBuilder builder(dockspace_id);
          layoutBuilderCb(builder);

          ImGui::DockBuilderFinish(dockspace_id);
        }
        layoutNeedsInit = false;
      }

      ImGui::DockSpaceOverViewport(dockspace_id, viewport,
                                   ImGuiDockNodeFlags_PassthruCentralNode);

      processMessageBoxes();

      for (auto *win : activeWindows)
      {
        win->render();
      }

      activeWindows.erase(
          std::remove_if(activeWindows.begin(), activeWindows.end(),
                         [](const auto *win)
                         { return !win->isWindowOpen(); }),
          activeWindows.end());

      if (mainLoopCallback)
      {
        mainLoopCallback();
      }

      ImGui::Render();
      if (g_use_opengl)
      {
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(45 / 255.0f, 45 / 255.0f, 45 / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(g_window);
      }
      else
      {
        SDL_SetRenderDrawColor(g_renderer, 45, 45, 45, 255);
        SDL_RenderClear(g_renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_renderer);
        SDL_RenderPresent(g_renderer);
      }

      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
      {
        if (g_use_opengl)
        {
          SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
          SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
          ImGui::UpdatePlatformWindows();
          ImGui::RenderPlatformWindowsDefault();
          SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
        else
        {
          ImGui::UpdatePlatformWindows();
          ImGui::RenderPlatformWindowsDefault();
        }
      }
    }

    App::shutdown();
  }

  void App::quit() { g_running = false; }

  void App::assertMainThread()
  {
    if (std::this_thread::get_id() != mainThreadId)
    {
      throw std::runtime_error(
          "MUI Error: UI interaction outside of the main thread.");
    }
  }

  void App::shutdown()
  {
    ShutdownShadows();
    // No need to reset unique_ptr, as platformService is now a raw pointer to a
    // static object.
  }

  void App::queueMain(std::function<void()> callback)
  {
    if (!callback)
      return;
    std::lock_guard<std::mutex> lock(m_mainQueueMutex);
    m_mainQueue.emplace_back(std::move(callback));
  }

  void App::drainMainQueue()
  {
    std::vector<std::function<void()>> currentQueue;
    {
      std::lock_guard<std::mutex> lock(m_mainQueueMutex);
      if (m_mainQueue.empty())
        return;
      std::swap(currentQueue, m_mainQueue);
    }

    for (auto &task : currentQueue)
    {
      task();
    }
  }

  void App::addMessageBox(ActiveMessageBox &&mb)
  {
    activeMessageBoxes.push_back(std::move(mb));
  }

  void App::processMessageBoxes()
  {
    if (activeMessageBoxes.empty())
    {
      return;
    }

    // Process only the first message box in the queue to show them sequentially.
    auto &mb = activeMessageBoxes.front();

    // Make the ID unique in case multiple popups with the same title are queued.
    std::string popup_id =
        mb.title + "##msgbox" + std::to_string(activeMessageBoxes.size());
    if (mb.open_popup)
    {
      ImGui::OpenPopup(popup_id.c_str());
      mb.open_popup = false;
    }

    // Center the popup on the screen when it first appears.
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing,
                            ImVec2(0.5f, 0.5f));

    // Set size constraints for the popup.
    ImVec2 min_size = ImVec2(350, 0);
    ImGui::SetNextWindowSizeConstraints(min_size, ImVec2(600, FLT_MAX));

    bool isOpen = true;
    if (ImGui::BeginPopupModal(popup_id.c_str(), &isOpen,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
      Dialogs::renderMessageBox(mb, isOpen);
      ImGui::EndPopup();
    }

    if (!isOpen)
    {
      activeMessageBoxes.erase(activeMessageBoxes.begin());
    }
  }

  void App::setTheme(ThemeType type)
  {
    if (currentTheme == type && !useTomlTheme)
      return;              // No change needed for base theme
    resetColorModifiers(); // Reset modifiers when changing base theme
    currentTheme = type;
    useTomlTheme = false;
    dpiNeedsUpdate =
        true; // Signal the main loop to reapply theme and rebuild fonts
  }

  void App::setTheme(const std::string &filepath, const std::string &themeName)
  {
    if (useTomlTheme && currentThemeFile == filepath &&
        currentThemeName == themeName)
      return;              // No change needed for TOML theme
    resetColorModifiers(); // Reset modifiers when changing TOML theme
    currentThemeFile = filepath;
    currentThemeName = themeName;
    useTomlTheme = true;
    dpiNeedsUpdate =
        true; // Signal the main loop to reapply theme and rebuild fonts
  }

  void App::setTheme(const std::string &themeName)
  {
    setTheme(filepath + "themes.toml", themeName);
  }

  std::vector<std::string> App::getAvailableThemes()
  {
    return Theme::getAvailableThemes(filepath + "themes.toml");
  }

  SDL_GLContext App::getGLContext() { return glContext; }

  void App::resetColorModifiers()
  {
    s_applyGrayscale = false;
    s_applyComplementary = false;
    s_applySepia = false;
    s_applyInvert = false;
    dpiNeedsUpdate = true;
  }

  void App::setApplyGrayscale(bool apply)
  {
    s_applyGrayscale = apply;
    dpiNeedsUpdate = true;
  }

  bool App::getApplyGrayscale() { return s_applyGrayscale; }

  void App::setApplyComplementary(bool apply)
  {
    s_applyComplementary = apply;
    dpiNeedsUpdate = true;
  }

  bool App::getApplyComplementary() { return s_applyComplementary; }

  void App::setApplySepia(bool apply)
  {
    s_applySepia = apply;
    dpiNeedsUpdate = true;
  }

  bool App::getApplySepia() { return s_applySepia; }

  void App::setApplyInvert(bool apply)
  {
    s_applyInvert = apply;
    dpiNeedsUpdate = true;
  }

  bool App::getApplyInvert() { return s_applyInvert; }
} // namespace mui
