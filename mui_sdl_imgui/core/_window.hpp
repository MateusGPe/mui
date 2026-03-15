// core/window.hpp
#pragma once
#include "../widgets/control.hpp"
#include <string>
#include <functional>
#include <memory>
#include <imgui.h>

namespace mui
{
    class Window;
    using WindowPtr = std::shared_ptr<Window>;

    // Assuming Window is NOT a child of Control since it acts as a top-level container in ImGui
    class Window : public std::enable_shared_from_this<Window>
    {
    protected:
        std::string title;
        int width;
        int height;
        bool isOpen;
        bool margined = true;
        bool borderless = false;
        ImGuiID dockId = 0;
        ControlPtr child;
        std::function<bool()> onClosingCb;

        WindowPtr self() { return shared_from_this(); }
        void verifyState() const;

    public:
        Window(const std::string &title, int width, int height, bool hasMenubar = false);
        virtual ~Window();

        static WindowPtr create(const std::string &title, int width, int height, bool hasMenubar = false)
        {
            return std::make_shared<Window>(title, width, height, hasMenubar);
        }

        void render();

        WindowPtr setChild(ControlPtr c);
        WindowPtr setMargined(bool m);
        WindowPtr setDockId(ImGuiID id);
        WindowPtr onClosing(std::function<bool()> cb);

        std::string getTitle() const;
        WindowPtr setTitle(const std::string &t);

        std::pair<int, int> getContentSize() const;
        WindowPtr setContentSize(int w, int h);

        bool getBorderless() const;
        WindowPtr setBorderless(bool b);
        bool getMargined() const;

        bool isWindowOpen() const;
    };
} // namespace mui