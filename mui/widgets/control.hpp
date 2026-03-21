#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <imgui.h>
#include <imgui_internal.h>
#include "../core/app.hpp"

namespace mui
{

    class IControl;
    using IControlPtr = std::shared_ptr<IControl>;

    // Helper struct to allow appending controls with an optional 'stretchy' flag.
    struct AppendedControl
    {
        IControlPtr control;
        bool stretchy;

        // Constructor for {control, stretchy} syntax
        inline AppendedControl(IControlPtr c, bool s) : control(std::move(c)), stretchy(s) {}
        // Constructor for {control} syntax, defaulting stretchy to false
        inline AppendedControl(IControlPtr c) : control(std::move(c)), stretchy(false) {}
    };

    // Non-templated base class for polymorphism.
    class IControl
    {
    public:
        virtual ~IControl() = default;
        virtual void render() = 0;
        virtual void onHandleDestroyed() = 0;
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

    template <class Derived>
    class Control : public IControl, public std::enable_shared_from_this<Derived>
    {
    protected:
        bool visible = true;
        bool enabled = true;
        bool ownsHandle = true;
        bool spanAvailWidth = false;
        float width = 0.0f;
        float height = 0.0f;
        ImVec2 position = {-1.0f, -1.0f}; // Use -1 to indicate auto-position
        bool useContainerWidth = false;
        ImVec2 minSize = {0.0f, 0.0f};
        ImVec2 maxSize = {FLT_MAX, FLT_MAX};

        std::string tooltip;

        // Static Global Shadow Defaults
        static bool s_defaultHasShadow;
        static ImVec2 s_defaultShadowOffset;
        static float s_defaultShadowBlur;
        static ImVec4 s_defaultShadowColor;
        static float s_defaultShadowRounding;

        // Shadow Properties
        bool hasShadow = s_defaultHasShadow;
        ImVec2 shadowOffset = s_defaultShadowOffset;
        float shadowBlur = s_defaultShadowBlur;
        ImVec4 shadowColor = s_defaultShadowColor;
        float shadowRounding = s_defaultShadowRounding; // -1 means use ImGui style default

        void renderTooltip()
        {
            if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            {
                ImGui::SetTooltip("%s", tooltip.c_str());
            }
        }

        // ALL WIDGETS MUST NOW IMPLEMENT THIS INSTEAD OF render()
        virtual void renderControl() = 0;

        std::shared_ptr<Derived> self()
        {
            return std::static_pointer_cast<Derived>(this->shared_from_this());
        }

    public:
        virtual ~Control() = default;

        // This is now final/non-virtual. It handles the shadow wrapper.
        void render() override final
        {
            if (!visible)
                return;

            // Apply absolute positioning if specified
            if (position.x >= 0.0f && position.y >= 0.0f)
            {
                ImGui::SetCursorPos(position);
            }

            ImGuiWindow *window = ImGui::GetCurrentWindowRead();
            if (window == nullptr || window->SkipItems)
                return;

            if (!hasShadow)
            {
                renderControl();
                return;
            }

            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            // 1. Split channels
            draw_list->ChannelsSplit(2);
            draw_list->ChannelsSetCurrent(1); // Foreground

            // 2. Render widget to get dimensions
            ImGui::BeginGroup();
            renderControl();
            ImGui::EndGroup();

            ImVec2 p_min = ImGui::GetItemRectMin();
            ImVec2 p_max = ImGui::GetItemRectMax();

            // 3. Switch to background to draw shadow
            draw_list->ChannelsSetCurrent(0);

            float rounding = shadowRounding < 0.0f ? ImGui::GetStyle().FrameRounding : shadowRounding;

            if (shadowBlur <= 0.0f)
            {
                // Hard/Flat Retro Shadow
                draw_list->AddRectFilled(
                    ImVec2(p_min.x + shadowOffset.x, p_min.y + shadowOffset.y),
                    ImVec2(p_max.x + shadowOffset.x, p_max.y + shadowOffset.y),
                    ImGui::GetColorU32(shadowColor),
                    rounding);
            }
            else
            {
                // Simulated Soft Shadow (Hack)
                // Warning: High iterations will spike draw calls. Keep between 3 and 5.
                const int iterations = 4;
                for (int i = 1; i <= iterations; ++i)
                {
                    float step = (shadowBlur / iterations) * i;
                    // Fade alpha outwards
                    float alpha = shadowColor.w * (1.0f - ((float)i / iterations));
                    ImU32 col = ImGui::GetColorU32(ImVec4(shadowColor.x, shadowColor.y, shadowColor.z, alpha));

                    draw_list->AddRectFilled(
                        ImVec2(p_min.x + shadowOffset.x - step, p_min.y + shadowOffset.y - step),
                        ImVec2(p_max.x + shadowOffset.x + step, p_max.y + shadowOffset.y + step),
                        col,
                        rounding + step);
                }
            }

            // 4. Merge
            draw_list->ChannelsMerge();
        }

        void onHandleDestroyed() override { ownsHandle = false; }
        void verifyState() const { App::assertMainThread(); }

        std::shared_ptr<Derived> show()
        {
            visible = true;
            return self();
        }
        std::shared_ptr<Derived> hide()
        {
            visible = false;
            return self();
        }

        std::shared_ptr<Derived> setEnabled(bool enabled)
        {
            this->enabled = enabled;
            return self();
        }
        bool isEnabled() const { return enabled; }

        std::shared_ptr<Derived> releaseOwnership()
        {
            ownsHandle = false;
            return self();
        }
        std::shared_ptr<Derived> acquireOwnership()
        {
            ownsHandle = true;
            return self();
        }

        std::shared_ptr<Derived> setTooltip(const std::string &t)
        {
            tooltip = t;
            return self();
        }
        std::shared_ptr<Derived> setSpanAvailWidth(bool span)
        {
            spanAvailWidth = span;
            return self();
        }

        std::shared_ptr<Derived> setUseContainerWidth(bool use)
        {
            useContainerWidth = use;
            return self();
        }

        std::shared_ptr<Derived> setSize(float w, float h)
        {
            width = w;
            height = h;
            return self();
        }

        ImVec2 getSize() const
        {
            return {width, height};
        }

        std::shared_ptr<Derived> setPosition(float x, float y)
        {
            position = {x, y};
            return self();
        }

        ImVec2 getPosition() const
        {
            return position;
        }
        std::shared_ptr<Derived> setX(float x)
        {
            position.x = x;
            return self();
        }
        std::shared_ptr<Derived> setY(float y)
        {
            position.y = y;
            return self();
        }
        float getX() const { return position.x; }
        float getY() const { return position.y; }
        std::shared_ptr<Derived> setWidth(float w)
        {
            width = w;
            return self();
        }
        std::shared_ptr<Derived> setHeight(float h)
        {
            height = h;
            return self();
        }
        float getWidth() const { return width; }
        float getHeight() const { return height; }
        // Shadow Configuration
        std::shared_ptr<Derived> setShadow(bool enable, ImVec2 offset = ImVec2(2, 2), float blur = 4.0f, ImVec4 col = ImVec4(0, 0, 0, 0.25f), float rounding = -1.0f)
        {
            hasShadow = enable;
            shadowOffset = offset;
            shadowBlur = blur;
            shadowColor = col;
            shadowRounding = rounding;
            return self();
        }

        std::shared_ptr<Derived> defaultShadow(bool enable = true)
        {
            hasShadow = s_defaultHasShadow || enable;
            shadowOffset = s_defaultShadowOffset;
            shadowBlur = s_defaultShadowBlur;
            shadowColor = s_defaultShadowColor;
            shadowRounding = s_defaultShadowRounding;
            return self();
        }

        // Global defaults setter
        static void setGlobalShadowDefaults(bool enable, ImVec2 offset = ImVec2(2, 2), float blur = 4.0f, ImVec4 col = ImVec4(0, 0, 0, 0.25f), float rounding = -1.0f)
        {
            s_defaultHasShadow = enable;
            s_defaultShadowOffset = offset;
            s_defaultShadowBlur = blur;
            s_defaultShadowColor = col;
            s_defaultShadowRounding = rounding;
        }

        std::shared_ptr<Derived> addSize(float w, float h)
        {
            width += w;
            height += h;
            return self();
        }

        std::shared_ptr<Derived> scaleSize(float factor)
        {
            width *= factor;
            height *= factor;
            return self();
        }

        std::shared_ptr<Derived> scaleSize(float factorX, float factorY)
        {
            width *= factorX;
            height *= factorY;
            return self();
        }

        std::shared_ptr<Derived> addPosition(float x, float y)
        {
            // If auto-positioning, start from 0,0 before adding.
            if (position.x < 0.0f)
                position.x = 0.0f;
            if (position.y < 0.0f)
                position.y = 0.0f;
            position.x += x;
            position.y += y;
            return self();
        }

        // Note: Widgets must internally respect these constraints during their renderControl implementation.
        std::shared_ptr<Derived> setMinSize(float w, float h)
        {
            minSize = {w, h};
            return self();
        }
        std::shared_ptr<Derived> setMaxSize(float w, float h)
        {
            maxSize = {w, h};
            return self();
        }
        ImVec2 getMinSize() const { return minSize; }
        ImVec2 getMaxSize() const { return maxSize; }

        std::shared_ptr<Derived> setMinWidth(float w)
        {
            minSize.x = w;
            return self();
        }
        std::shared_ptr<Derived> setMinHeight(float h)
        {
            minSize.y = h;
            return self();
        }
        std::shared_ptr<Derived> setMaxWidth(float w)
        {
            maxSize.x = w;
            return self();
        }
        std::shared_ptr<Derived> setMaxHeight(float h)
        {
            maxSize.y = h;
            return self();
        }
    };

    template <class T>
    bool Control<T>::s_defaultHasShadow = false;
    template <class T>
    ImVec2 Control<T>::s_defaultShadowOffset = ImVec2(2.0f, 2.0f);
    template <class T>
    float Control<T>::s_defaultShadowBlur = 4.0f;
    template <class T>
    ImVec4 Control<T>::s_defaultShadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.25f);
    template <class T>
    float Control<T>::s_defaultShadowRounding = -1.0f;

} // namespace mui
