#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{

  // --- Window ---
  class Window : public Control
  {
  private:
    uiWindow *win = nullptr;
    std::shared_ptr<Control> child; // Keeps the child C++ object alive
    std::function<bool()> onClosingCb;
    std::function<void()> onPositionChangedCb;
    std::function<void()> onContentSizeChangedCb;
    std::function<void()> onFocusChangedCb;

    static int onClosingStub(uiWindow *w, void *data);
    static void onPositionChangedStub(uiWindow *w, void *data);
    static void onContentSizeChangedStub(uiWindow *w, void *data);
    static void onFocusChangedStub(uiWindow *w, void *data);

  protected:
    void onHandleDestroyed() override;

  public:
    Window(const std::string &title, int width, int height,
           bool hasMenubar = false);

    template <typename... Args>
    static std::shared_ptr<Window> create(Args &&...args)
    {
      return std::make_shared<Window>(std::forward<Args>(args)...);
    }

    std::shared_ptr<Window> setChild(std::shared_ptr<Control> child);
    std::shared_ptr<Window> setMargined(bool margined);
    std::shared_ptr<Window> onClosing(std::function<bool()> cb);
    std::shared_ptr<Window> onPositionChanged(std::function<void()> cb);
    std::shared_ptr<Window> onContentSizeChanged(std::function<void()> cb);
    std::shared_ptr<Window> onFocusChanged(std::function<void()> cb);
    bool getFocused() const;
    std::string getTitle() const;
    std::shared_ptr<Window> setTitle(const std::string &title);
    std::pair<int, int> getContentSize() const;
    std::shared_ptr<Window> setContentSize(int width, int height);
    bool getFullscreen() const;
    std::shared_ptr<Window> setFullscreen(bool fullscreen);
    bool getBorderless() const;
    std::shared_ptr<Window> setBorderless(bool borderless);
    bool getResizeable() const;
    std::shared_ptr<Window> setResizeable(bool resizeable);
    std::pair<int, int> getPosition() const;
    std::shared_ptr<Window> setPosition(int x, int y);
    bool getMargined() const;
  };

} // namespace mui