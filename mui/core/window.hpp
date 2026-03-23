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

  using ImGuiWindowFlags = int;

  using Identifier = unsigned int;
  class Window : public Control<Window>
  {
  private:
    std::string title;
    int width, height;
    bool isOpen = true;
    bool margined = true;
    IControlPtr child;
    bool noTabBar = false;
    std::function<bool()> onClosingCb;
    bool _needs_focus;
    Identifier dockId;
    ImGuiWindowFlags m_flags;

  protected:
    Window(const std::string &title, int width, int height, bool hasMenubar = false);

  public:
    ~Window();

    void renderControl() override;
    WindowPtr setChild(IControlPtr child);
    WindowPtr setMargined(bool margined);
    WindowPtr onClosing(std::function<bool()> cb);
    WindowPtr setNoTabBar(bool b);
    bool getNoTabBar() const;
    std::string getTitle() const;
    WindowPtr setTitle(const std::string &title);
    std::pair<int, int> getContentSize() const;
    WindowPtr setContentSize(int width, int height);
    bool getBorderless() const;
    WindowPtr setBorderless(bool borderless);
    bool getMargined() const;
    bool isWindowOpen() const;
    WindowPtr setDockId(Identifier id);

    void close();
    WindowPtr focus();

    WindowPtr setHasMenubar(bool b);
    bool getHasMenubar() const;
    WindowPtr setResizable(bool b);
    bool getResizable() const;
    WindowPtr setMovable(bool b);
    bool getMovable() const;
    WindowPtr setCollapsible(bool b);
    bool getCollapsible() const;
    WindowPtr setHasTitlebar(bool b);
    bool getHasTitlebar() const;

    WindowPtr setScrollbar(bool b);
    bool getScrollbar() const;
    WindowPtr setScrollWithMouse(bool b);
    bool getScrollWithMouse() const;
    WindowPtr setAlwaysAutoResize(bool b);
    bool getAlwaysAutoResize() const;
    WindowPtr setBackground(bool b);
    bool getBackground() const;
    WindowPtr setSavedSettings(bool b);
    bool getSavedSettings() const;
    WindowPtr setMouseInputs(bool b);
    bool getMouseInputs() const;
    WindowPtr setHorizontalScrollbar(bool b);
    bool getHorizontalScrollbar() const;
    WindowPtr setFocusOnAppearing(bool b);
    bool getFocusOnAppearing() const;
    WindowPtr setBringToFrontOnFocus(bool b);
    bool getBringToFrontOnFocus() const;
    WindowPtr setAlwaysVerticalScrollbar(bool b);
    bool getAlwaysVerticalScrollbar() const;
    WindowPtr setAlwaysHorizontalScrollbar(bool b);
    bool getAlwaysHorizontalScrollbar() const;
    WindowPtr setNavInputs(bool b);
    bool getNavInputs() const;
    WindowPtr setNavFocus(bool b);
    bool getNavFocus() const;
    WindowPtr setUnsavedDocument(bool b);
    bool getUnsavedDocument() const;
    WindowPtr setDocking(bool b);
    bool getDocking() const;

    static WindowPtr create(const std::string &title, int width, int height, bool hasMenubar = false) 
    { 
        return std::shared_ptr<Window>(new Window(title, width, height, hasMenubar)); 
    }
  };

} // namespace mui