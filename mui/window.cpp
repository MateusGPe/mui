#include "window.hpp"
#include "app.hpp"

namespace mui
{

  // --- Window ---
  int Window::onClosingStub(uiWindow *w, void *data)
  {
    auto self = static_cast<Window *>(data);
    // If no callback is provided, default to closing the window.
    bool shouldClose = self->onClosingCb ? self->onClosingCb() : true;
    if (shouldClose)
    {
      // libui destroys the window and its children on returning 1.
      // We must cascade the destruction state through the C++ tree to prevent
      // double-freeing.
      self->onHandleDestroyed();
    }
    return shouldClose ? 1 : 0;
  }

  Window::Window(const std::string &title, int width, int height,
                 bool hasMenubar)
  {
    App::assertMainThread();
    win = uiNewWindow(title.c_str(), width, height, hasMenubar ? 1 : 0);
    handle = uiControl(win);
    uiWindowOnClosing(win, onClosingStub, this);
    uiWindowOnPositionChanged(win, onPositionChangedStub, this);
    uiWindowOnContentSizeChanged(win, onContentSizeChangedStub, this);
    uiWindowOnFocusChanged(win, onFocusChangedStub, this);
  }

  void Window::onFocusChangedStub(uiWindow *w, void *data)
  {
    auto self = static_cast<Window *>(data);
    if (self->onFocusChangedCb)
      self->onFocusChangedCb();
  }

  WindowPtr Window::onFocusChanged(std::function<void()> cb)
  {
    verifyState();
    onFocusChangedCb = std::move(cb);
    return self();
  }

  bool Window::getFocused() const
  {
    verifyState();
    return uiWindowFocused(win);
  }


  void Window::onContentSizeChangedStub(uiWindow *w, void *data)
  {
    auto self = static_cast<Window *>(data);
    if (self->onContentSizeChangedCb)
      self->onContentSizeChangedCb();
  }

  WindowPtr Window::onContentSizeChanged(std::function<void()> cb)
  {
    verifyState();
    onContentSizeChangedCb = std::move(cb);
    return self();
  }


  void Window::onPositionChangedStub(uiWindow *w, void *data)
  {
    auto self = static_cast<Window *>(data);
    if (self->onPositionChangedCb)
      self->onPositionChangedCb();
  }

  WindowPtr Window::onPositionChanged(std::function<void()> cb)
  {
    verifyState();
    onPositionChangedCb = std::move(cb);
    return self();
  }

  void Window::onHandleDestroyed()
  {
    Control::onHandleDestroyed();
    win = nullptr;
    if (child)
      child->onHandleDestroyed();
  }

  WindowPtr Window::setChild(ControlPtr c)
  {
    verifyState();
    child = c;
    uiWindowSetChild(win, c->getHandle());
    c->releaseOwnership(); // libui takes memory ownership
    return self();
  }

  WindowPtr Window::setMargined(bool margined)
  {
    verifyState();
    uiWindowSetMargined(win, margined ? 1 : 0);
    return self();
  }

  WindowPtr Window::onClosing(std::function<bool()> cb)
  {
    verifyState();
    onClosingCb = std::move(cb);
    return self();
  }

  std::string Window::getTitle() const
  {
    verifyState();
    return UiText(uiWindowTitle(win));
  }

  WindowPtr Window::setTitle(const std::string &title)
  {
    verifyState();
    uiWindowSetTitle(win, title.c_str());
    return self();
  }

  std::pair<int, int> Window::getContentSize() const
  {
    verifyState();
    int width, height;
    uiWindowContentSize(win, &width, &height);
    return {width, height};
  }

  WindowPtr Window::setContentSize(int width, int height)
  {
    verifyState();
    uiWindowSetContentSize(win, width, height);
    return self();
  }

  bool Window::getFullscreen() const
  {
    verifyState();
    return uiWindowFullscreen(win);
  }

  WindowPtr Window::setFullscreen(bool fullscreen)
  {
    verifyState();
    uiWindowSetFullscreen(win, fullscreen);
    return self();
  }

  bool Window::getBorderless() const
  {
    verifyState();
    return uiWindowBorderless(win);
  }

  WindowPtr Window::setBorderless(bool borderless)
  {
    verifyState();
    uiWindowSetBorderless(win, borderless);
    return self();
  }

  bool Window::getResizeable() const
  {
    verifyState();
    return uiWindowResizeable(win);
  }

  WindowPtr Window::setResizeable(bool resizeable)
  {
    verifyState();
    uiWindowSetResizeable(win, resizeable);
    return self();
  }

  std::pair<int, int> Window::getPosition() const
  {
    verifyState();
    int x, y;
    uiWindowPosition(win, &x, &y);
    return {x, y};
  }

  WindowPtr Window::setPosition(int x, int y)
  {
    verifyState();
    uiWindowSetPosition(win, x, y);
    return self();
  }

  bool Window::getMargined() const
  {
    verifyState();
    return uiWindowMargined(win);
  }

} // namespace mui