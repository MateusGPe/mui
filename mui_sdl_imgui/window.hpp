#pragma once
#include "control.hpp"
#include "core.hpp"
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace mui
{
  class Window;
  using WindowPtr = std::shared_ptr<Window>;

  class Window: public Control, public Chainable<Window>
  {
  private:
    std::string title;
    int width, height;
    bool isOpen = true;
    bool borderless = false;
    bool margined = true;
    ControlPtr child;
    std::function<bool()> onClosingCb;

  public:
    Window(const std::string &title, int width, int height, bool hasMenubar = false);
    ~Window();

    void render() override;
    WindowPtr setChild(ControlPtr child);
    WindowPtr setMargined(bool margined);
    WindowPtr onClosing(std::function<bool()> cb);
    std::string getTitle() const;
    WindowPtr setTitle(const std::string &title);
    std::pair<int, int> getContentSize() const;
    WindowPtr setContentSize(int width, int height);
    bool getBorderless() const;
    WindowPtr setBorderless(bool borderless);
    bool getMargined() const;
    bool isWindowOpen() const;
  };

} // namespace mui