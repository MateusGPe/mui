#pragma once
#include <string>
#include <memory>
#include <stdexcept>
#include <imgui.h>

namespace mui
{
    class Control;
    using ControlPtr = std::shared_ptr<Control>;
    // Helper struct to allow appending controls with an optional 'stretchy' flag.
    struct AppendedControl
    {
        std::shared_ptr<Control> control;
        bool stretchy;

        // Constructor for {control, stretchy} syntax
        inline AppendedControl(std::shared_ptr<Control> c, bool s) : control(std::move(c)), stretchy(s) {}
        // Constructor for {control} syntax, defaulting stretchy to false
        inline AppendedControl(std::shared_ptr<Control> c) : control(std::move(c)), stretchy(false) {}
    };

    // Helper function to append multiple controls to a Box container (VBox, HBox).
    template <typename T>
    inline static void append_all(const std::shared_ptr<T> &container, std::initializer_list<AppendedControl> items)
    {
        for (const auto &item : items)
        {
            container->append(item.control, item.stretchy);
        }
    }
    class Control : public std::enable_shared_from_this<Control>
    {
    protected:
        bool visible = true;
        bool enabled = true;
        bool ownsHandle = true;
        std::string tooltip;

        // Shadow Properties
        bool hasShadow = false;
        ImVec2 shadowOffset = ImVec2(2.0f, 2.0f);
        float shadowBlur = 4.0f;
        ImVec4 shadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.25f);
        float shadowRounding = -1.0f; // -1 means use ImGui style default

        void renderTooltip();

        // ALL WIDGETS MUST NOW IMPLEMENT THIS INSTEAD OF render()
        virtual void renderControl() = 0;

    public:
        virtual ~Control();

        // This is now final/non-virtual. It handles the shadow wrapper.
        void render();

        virtual void onHandleDestroyed();
        void verifyState() const;

        void show();
        void hide();

        void setEnabled(bool enabled);
        bool isEnabled() const;

        void releaseOwnership();
        void acquireOwnership();

        ControlPtr setTooltip(const std::string &t);

        // Shadow Configuration
        ControlPtr setShadow(bool enable, ImVec2 offset = ImVec2(2, 2), float blur = 4.0f, ImVec4 col = ImVec4(0, 0, 0, 0.25f), float rounding = -1.0f);
    };
} // namespace mui