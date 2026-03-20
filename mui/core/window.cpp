#include "window.hpp"
#include "app.hpp"
#include <imgui.h>
#include <algorithm>

namespace mui
{
    Window::Window(const std::string &title, int width, int height, bool hasMenubar)
        : title(title), width(width), height(height), isOpen(true), dockId(0)
    {
        App::assertMainThread();
        App::activeWindows.push_back(this);
    }

    Window::~Window()
    {
        auto it = std::find(App::activeWindows.begin(), App::activeWindows.end(), this);
        if (it != App::activeWindows.end())
        {
            App::activeWindows.erase(it);
        }
    }

    void Window::renderControl()
    {
        if (!isOpen)
            return;

        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);

        if (dockId != 0)
        {
            ImGui::SetNextWindowDockID(dockId, ImGuiCond_FirstUseEver);
        }

        if (!margined)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        }

        bool openCache = isOpen;
        if (ImGui::Begin(title.c_str(), &openCache, borderless ? ImGuiWindowFlags_NoDecoration : 0))
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

    WindowPtr Window::setChild(ControlPtr c)
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

    WindowPtr Window::setDockId(ImGuiID id)
    {
        verifyState();
        dockId = id;
        return self();
    }

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
    bool Window::getBorderless() const { return borderless; }
    WindowPtr Window::setBorderless(bool b)
    {
        borderless = b;
        return self();
    }
    bool Window::getMargined() const { return margined; }

    bool Window::isWindowOpen() const { return isOpen; }

} // namespace mui
