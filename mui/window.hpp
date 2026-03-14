#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{
  class Window;
  using WindowPtr = std::shared_ptr<Window>;
  // --- Window ---
  class Window: public Control, public Chainable<Window>
  {
  private:
    uiWindow *win = nullptr;
    ControlPtr child; // Keeps the child C++ object alive
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
    static WindowPtr create(Args &&...args)
    {
      return std::make_shared<Window>(std::forward<Args>(args)...);
    }

    WindowPtr setChild(ControlPtr child);
    WindowPtr setMargined(bool margined);
    WindowPtr onClosing(std::function<bool()> cb);
    WindowPtr onPositionChanged(std::function<void()> cb);
    WindowPtr onContentSizeChanged(std::function<void()> cb);
    WindowPtr onFocusChanged(std::function<void()> cb);
    bool getFocused() const;
    std::string getTitle() const;
    WindowPtr setTitle(const std::string &title);
    std::pair<int, int> getContentSize() const;
    WindowPtr setContentSize(int width, int height);
    bool getFullscreen() const;
    WindowPtr setFullscreen(bool fullscreen);
    bool getBorderless() const;
    WindowPtr setBorderless(bool borderless);
    bool getResizeable() const;
    WindowPtr setResizeable(bool resizeable);
    std::pair<int, int> getPosition() const;
    WindowPtr setPosition(int x, int y);
    bool getMargined() const;
  };

} // namespace mui