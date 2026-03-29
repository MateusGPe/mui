#include "window.hpp"
#include "app.hpp"
#include <algorithm>
#include <imgui.h>

namespace mui
{
    Window::Window(const std::string &title, int width, int height, bool hasMenubar)
        : title(title), width(width), height(height), isOpen(true),
          _needs_focus(false), dockId(0)
    {
        // Initialize flags. By default, most features are enabled (meaning the "No"
        // flags are not set).
        m_flags = hasMenubar ? ImGuiWindowFlags_MenuBar : ImGuiWindowFlags_None;
        App::assertMainThread();
        App::activeWindows.push_back(this);
        inlineShadowEnabled = false;
    }

    Window::~Window()
    {
        auto it =
            std::find(App::activeWindows.begin(), App::activeWindows.end(), this);
        if (it != App::activeWindows.end())
        {
            App::activeWindows.erase(it);
        }
    }

    void Window::renderControl()
    {
        if (!isOpen)
            return;

        if (_needs_focus)
        {
            ImGui::SetNextWindowFocus();
            _needs_focus = false;
        }

        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);

        if (dockId != 0)
        {
            ImGui::SetNextWindowDockID(dockId, ImGuiCond_FirstUseEver);
        }

        if (!margined)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        }

        ImGuiWindowClass window_class;
        if (noTabBar)
        {
            window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
            ImGui::SetNextWindowClass(&window_class);
        }

        bool openCache = isOpen;
        if (ImGui::Begin(title.c_str(), &openCache, m_flags))
        {
            if (child)
            {
                child->render();
            }
        }
        ImGui::End();

        // Pop the style if it was pushed.
        if (!margined)
        {
            ImGui::PopStyleVar();
        }

        if (!openCache)
        {
            bool shouldClose = true;
            if (onClosingCb)
            {
                try
                {
                    shouldClose = onClosingCb();
                }
                catch (...)
                {
                    shouldClose = true;
                }
            }
            if (shouldClose)
            {
                isOpen = false;
            }
        }
    }

    WindowPtr Window::setChild(IControlPtr c)
    {
        verifyState();
        child = c;
        return self();
    }

    WindowPtr Window::setMargined(bool m)
    {
        verifyState();
        margined = m;
        return self();
    }

    WindowPtr Window::setDockId(Identifier id)
    {
        verifyState();
        dockId = id;
        return self();
    }

    WindowPtr Window::setNoTabBar(bool b)
    {
        verifyState();
        noTabBar = b;
        return self();
    }
    bool Window::getNoTabBar() const { return noTabBar; }

    WindowPtr Window::onClosing(std::function<bool()> cb)
    {
        onClosingCb = std::move(cb);
        return self();
    }
    std::string Window::getTitle() const { return title; }
    WindowPtr Window::setTitle(const std::string &t)
    {
        title = t;
        return self();
    }
    std::pair<int, int> Window::getContentSize() const { return {width, height}; }
    WindowPtr Window::setContentSize(int w, int h)
    {
        width = w;
        height = h;
        return self();
    }
    bool Window::getBorderless() const
    {
        // A window is borderless if its constituent properties are all set to
        // generate no decoration. Note: ImGuiWindowFlags_NoDecoration also includes
        // NoScrollbar and NoCollapse.
        return (m_flags & ImGuiWindowFlags_NoDecoration) ==
               ImGuiWindowFlags_NoDecoration;
    }
    WindowPtr Window::setBorderless(bool b)
    {
        // This is a utility to set/unset the composite ImGuiWindowFlags_NoDecoration
        // flag.
        if (b)
            m_flags |= ImGuiWindowFlags_NoDecoration;
        else
            m_flags &= ~ImGuiWindowFlags_NoDecoration;
        return self();
    }

    bool Window::getMargined() const { return margined; }

    bool Window::isWindowOpen() const { return isOpen; }

    void Window::close() { isOpen = false; }

    WindowPtr Window::focus()
    {
        _needs_focus = true;
        return self();
    }

    WindowPtr Window::setHasMenubar(bool b)
    {
        if (b)
            m_flags |= ImGuiWindowFlags_MenuBar;
        else
            m_flags &= ~ImGuiWindowFlags_MenuBar;
        return self();
    }

    bool Window::getHasMenubar() const
    {
        return (m_flags & ImGuiWindowFlags_MenuBar) != 0;
    }

    WindowPtr Window::setResizable(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoResize;
        else
            m_flags |= ImGuiWindowFlags_NoResize;
        return self();
    }

    bool Window::getResizable() const
    {
        return !(m_flags & ImGuiWindowFlags_NoResize);
    }

    WindowPtr Window::setMovable(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoMove;
        else
            m_flags |= ImGuiWindowFlags_NoMove;
        return self();
    }

    bool Window::getMovable() const { return !(m_flags & ImGuiWindowFlags_NoMove); }

    WindowPtr Window::setCollapsible(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoCollapse;
        else
            m_flags |= ImGuiWindowFlags_NoCollapse;
        return self();
    }

    bool Window::getCollapsible() const
    {
        return !(m_flags & ImGuiWindowFlags_NoCollapse);
    }

    WindowPtr Window::setHasTitlebar(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoTitleBar;
        else
            m_flags |= ImGuiWindowFlags_NoTitleBar;
        return self();
    }

    bool Window::getHasTitlebar() const
    {
        return !(m_flags & ImGuiWindowFlags_NoTitleBar);
    }

    WindowPtr Window::setScrollbar(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoScrollbar;
        else
            m_flags |= ImGuiWindowFlags_NoScrollbar;
        return self();
    }
    bool Window::getScrollbar() const
    {
        return !(m_flags & ImGuiWindowFlags_NoScrollbar);
    }
    WindowPtr Window::setScrollWithMouse(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoScrollWithMouse;
        else
            m_flags |= ImGuiWindowFlags_NoScrollWithMouse;
        return self();
    }
    bool Window::getScrollWithMouse() const
    {
        return !(m_flags & ImGuiWindowFlags_NoScrollWithMouse);
    }
    WindowPtr Window::setAlwaysAutoResize(bool b)
    {
        if (b)
            m_flags |= ImGuiWindowFlags_AlwaysAutoResize;
        else
            m_flags &= ~ImGuiWindowFlags_AlwaysAutoResize;
        return self();
    }
    bool Window::getAlwaysAutoResize() const
    {
        return (m_flags & ImGuiWindowFlags_AlwaysAutoResize) != 0;
    }
    WindowPtr Window::setBackground(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoBackground;
        else
            m_flags |= ImGuiWindowFlags_NoBackground;
        return self();
    }
    bool Window::getBackground() const
    {
        return !(m_flags & ImGuiWindowFlags_NoBackground);
    }
    WindowPtr Window::setSavedSettings(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoSavedSettings;
        else
            m_flags |= ImGuiWindowFlags_NoSavedSettings;
        return self();
    }
    bool Window::getSavedSettings() const
    {
        return !(m_flags & ImGuiWindowFlags_NoSavedSettings);
    }
    WindowPtr Window::setMouseInputs(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoMouseInputs;
        else
            m_flags |= ImGuiWindowFlags_NoMouseInputs;
        return self();
    }
    bool Window::getMouseInputs() const
    {
        return !(m_flags & ImGuiWindowFlags_NoMouseInputs);
    }
    WindowPtr Window::setHorizontalScrollbar(bool b)
    {
        if (b)
            m_flags |= ImGuiWindowFlags_HorizontalScrollbar;
        else
            m_flags &= ~ImGuiWindowFlags_HorizontalScrollbar;
        return self();
    }
    bool Window::getHorizontalScrollbar() const
    {
        return (m_flags & ImGuiWindowFlags_HorizontalScrollbar) != 0;
    }
    WindowPtr Window::setFocusOnAppearing(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoFocusOnAppearing;
        else
            m_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
        return self();
    }
    bool Window::getFocusOnAppearing() const
    {
        return !(m_flags & ImGuiWindowFlags_NoFocusOnAppearing);
    }
    WindowPtr Window::setBringToFrontOnFocus(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoBringToFrontOnFocus;
        else
            m_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        return self();
    }
    bool Window::getBringToFrontOnFocus() const
    {
        return !(m_flags & ImGuiWindowFlags_NoBringToFrontOnFocus);
    }
    WindowPtr Window::setAlwaysVerticalScrollbar(bool b)
    {
        if (b)
            m_flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
        else
            m_flags &= ~ImGuiWindowFlags_AlwaysVerticalScrollbar;
        return self();
    }
    bool Window::getAlwaysVerticalScrollbar() const
    {
        return (m_flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) != 0;
    }
    WindowPtr Window::setAlwaysHorizontalScrollbar(bool b)
    {
        if (b)
            m_flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
        else
            m_flags &= ~ImGuiWindowFlags_AlwaysHorizontalScrollbar;
        return self();
    }
    bool Window::getAlwaysHorizontalScrollbar() const
    {
        return (m_flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) != 0;
    }
    WindowPtr Window::setNavInputs(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoNavInputs;
        else
            m_flags |= ImGuiWindowFlags_NoNavInputs;
        return self();
    }
    bool Window::getNavInputs() const
    {
        return !(m_flags & ImGuiWindowFlags_NoNavInputs);
    }
    WindowPtr Window::setNavFocus(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoNavFocus;
        else
            m_flags |= ImGuiWindowFlags_NoNavFocus;
        return self();
    }
    bool Window::getNavFocus() const
    {
        return !(m_flags & ImGuiWindowFlags_NoNavFocus);
    }
    WindowPtr Window::setUnsavedDocument(bool b)
    {
        if (b)
            m_flags |= ImGuiWindowFlags_UnsavedDocument;
        else
            m_flags &= ~ImGuiWindowFlags_UnsavedDocument;
        return self();
    }
    bool Window::getUnsavedDocument() const
    {
        return (m_flags & ImGuiWindowFlags_UnsavedDocument) != 0;
    }
    WindowPtr Window::setDocking(bool b)
    {
        if (b)
            m_flags &= ~ImGuiWindowFlags_NoDocking;
        else
            m_flags |= ImGuiWindowFlags_NoDocking;
        return self();
    }
    bool Window::getDocking() const
    {
        return !(m_flags & ImGuiWindowFlags_NoDocking);
    }

} // namespace mui
