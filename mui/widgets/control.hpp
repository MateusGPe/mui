// widgets/control.hpp
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <imgui.h>
#include <imgui_internal.h>
#include "../core/app.hpp"
#include "../core/signal.hpp"
#include "../core/shadows.hpp" // Added ImGuiShadows include
#include "../core/stylesheet.hpp"
#include <optional>

namespace mui
{
    // Forward declaration for ScopedControlID
    class IControl;

    struct ScopedControlID
    {
        bool pushed = false;
        ScopedControlID(IControl *ctrl);
        ~ScopedControlID()
        {
            if (pushed)
                ImGui::PopID();
        }
    };

    using IControlPtr = std::shared_ptr<IControl>;

    // Non-templated base class for polymorphism.
    class IControl
    {
    protected:
        // Centralized connection management for all controls
        // Ensures safe lifecycle and automatic disconnection upon destruction
        std::vector<mui::Connection> m_connections;

    public:
        virtual ~IControl() = default;
        virtual void render() = 0;
        virtual void onHandleDestroyed() = 0;
        virtual std::string getID() const = 0;
        virtual bool isVisible() const = 0;

        void clearConnections() { m_connections.clear(); }
    };

    template <class Derived>
    class Control : public IControl, public std::enable_shared_from_this<Derived>
    {
    protected:
        std::vector<std::string> m_classes;
        std::vector<const StyleBlock *> m_cachedStyles;
        unsigned int m_styleVersion = 0;

        // Derived classes MUST implement this to identify themselves (e.g., return "Button")
        virtual std::string getTypeName() const = 0;

        void updateStyleCache()
        {
            m_cachedStyles.clear();
            if (auto b = StyleSheet::get(getTypeName()))
                m_cachedStyles.push_back(b);
            for (const auto &cls : m_classes)
                if (auto b = StyleSheet::get("." + cls))
                    m_cachedStyles.push_back(b);
            if (!m_customId.empty())
                if (auto b = StyleSheet::get("#" + m_customId))
                    m_cachedStyles.push_back(b);

            // Resolve Shadow Properties: Globals -> Stylesheet -> Inline
            m_resolvedHasShadow = StyleSheet::s_defaultHasShadow;
            m_resolvedShadowOffset = StyleSheet::s_defaultShadowOffset;
            m_resolvedShadowBlur = StyleSheet::s_defaultShadowBlur;
            m_resolvedShadowColor = StyleSheet::s_defaultShadowColor;
            m_resolvedShadowRounding = StyleSheet::s_defaultShadowRounding;
            m_resolvedShadowFillBackground = StyleSheet::s_defaultShadowFillBackground;
            m_resolvedShadowThickness = StyleSheet::s_defaultShadowThickness;

            for (const auto *block : m_cachedStyles)
            {
                if (block->shadowEnabled)
                    m_resolvedHasShadow = *block->shadowEnabled;
                else
                    continue;
                if (block->shadowOffset)
                    m_resolvedShadowOffset = *block->shadowOffset;
                if (block->shadowBlur)
                    m_resolvedShadowBlur = *block->shadowBlur;
                if (block->shadowColor)
                    m_resolvedShadowColor = *block->shadowColor;
                if (block->shadowRounding)
                    m_resolvedShadowRounding = *block->shadowRounding;
                if (block->shadowFillBackground)
                    m_resolvedShadowFillBackground = *block->shadowFillBackground;
                if (block->shadowThickness)
                    m_resolvedShadowThickness = *block->shadowThickness;
            }

            if (inlineShadowEnabled)
            {
                m_resolvedHasShadow = *inlineShadowEnabled;
                if (inlineShadowOffset)
                    m_resolvedShadowOffset = *inlineShadowOffset;
                if (inlineShadowBlur)
                    m_resolvedShadowBlur = *inlineShadowBlur;
                if (inlineShadowColor)
                    m_resolvedShadowColor = *inlineShadowColor;
                if (inlineShadowRounding)
                    m_resolvedShadowRounding = *inlineShadowRounding;
                if (inlineShadowFillBackground)
                    m_resolvedShadowFillBackground = *inlineShadowFillBackground;
                if (inlineShadowThickness)
                    m_resolvedShadowThickness = *inlineShadowThickness;
            }

            m_styleVersion = StyleSheet::getVersion();
        }
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
        std::string m_customId; // Stable ID to prevent ImGui state loss on reallocation

        // Shadow Properties (inline overrides)
        std::optional<bool> inlineShadowEnabled;
        std::optional<ImVec2> inlineShadowOffset;
        std::optional<float> inlineShadowBlur;
        std::optional<ImVec4> inlineShadowColor;
        std::optional<float> inlineShadowRounding;
        std::optional<bool> inlineShadowFillBackground;
        std::optional<float> inlineShadowThickness;

        // Cached Resolved Shadow Properties
        bool m_resolvedHasShadow = false;
        ImVec2 m_resolvedShadowOffset = {0.0f, 0.0f};
        float m_resolvedShadowBlur = 10.0f;
        ImVec4 m_resolvedShadowColor = {-1.0f, -1.0f, -1.0f, -1.0f};
        float m_resolvedShadowRounding = -1.0f;
        bool m_resolvedShadowFillBackground = true;
        float m_resolvedShadowThickness = -1.0f;

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

        std::shared_ptr<Derived> addClass(const std::string &cls)
        {
            m_classes.push_back(cls);
            m_styleVersion = 0; // Force update on next render
            return self();
        }

        std::shared_ptr<Derived> setID(const std::string &id)
        {
            m_customId = id;
            m_styleVersion = 0; // Force update on next render
            return self();
        }

        // This is now final/non-virtual. It handles the shadow wrapper.
        void render() override final
        {
            if (!visible)
                return;

            // Fast cache invalidation check
            if (m_styleVersion != StyleSheet::getVersion())
            {
                updateStyleCache();
            }

            // RAII struct to safely push and pop ImGui styles, immune to early returns
            struct ScopedStyleApplier
            {
                int colorPushes = 0;
                int varPushes = 0;
                ScopedStyleApplier(const std::vector<const StyleBlock *> &blocks)
                {
                    if (blocks.empty())
                        return;
                    for (const auto *block : blocks)
                    {
                        if (!block->colors.empty())
                        {
                            for (const auto &[col, val] : block->colors)
                            {
                                ImGui::PushStyleColor(col, val);
                            }
                            colorPushes += block->colors.size();
                        }
                        if (!block->floats.empty())
                        {
                            for (const auto &[var, val] : block->floats)
                            {
                                ImGui::PushStyleVar(var, val);
                            }
                            varPushes += block->floats.size();
                        }
                        if (!block->vec2s.empty())
                        {
                            for (const auto &[var, val] : block->vec2s)
                            {
                                ImGui::PushStyleVar(var, val);
                            }
                            varPushes += block->vec2s.size();
                        }
                    }
                }
                ~ScopedStyleApplier()
                {
                    if (varPushes > 0)
                        ImGui::PopStyleVar(varPushes);
                    if (colorPushes > 0)
                        ImGui::PopStyleColor(colorPushes);
                }
            } style_applier(m_cachedStyles);

            // Apply absolute positioning if specified
            if (position.x >= 0.0f && position.y >= 0.0f)
            {
                ImGui::SetCursorPos(position);
            }
            ImGuiWindow *window = ImGui::GetCurrentWindowRead();
            if (window == nullptr || window->SkipItems)
                return;

            if (!m_resolvedHasShadow || m_resolvedShadowThickness <= 0.0f)
            {
                renderControl();
                return;
            }

            ImTextureID shadow_tex = App::GetShadowTexture();
            if (!shadow_tex)
            {
                renderControl();
                return;
            }

            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            // 1. Split channels using a local splitter to safely allow nesting
            ImDrawListSplitter splitter;
            splitter.Split(draw_list, 2);
            splitter.SetCurrentChannel(draw_list, 1); // Foreground

            // 2. Render widget to get dimensions
            ImGui::BeginGroup();
            renderControl();
            ImGui::EndGroup();

            ImVec2 p_min = ImGui::GetItemRectMin();
            ImVec2 p_max = ImGui::GetItemRectMax();

            // 3. Switch to background to draw shadow
            splitter.SetCurrentChannel(draw_list, 0);
            float rounding = m_resolvedShadowRounding < 0.0f ? ImGui::GetStyle().FrameRounding : m_resolvedShadowRounding;
            const ImVec4 *shadow_uvs = App::GetShadowUVs();
            if (shadow_tex)
            {
                ImVec4 activeShadowColor = m_resolvedShadowColor;
                if (activeShadowColor.w < 0.0f)
                {
                    activeShadowColor = App::GetThemeShadowColor();
                }
                ImU32 col32 = ImGui::GetColorU32(activeShadowColor);
                ImGuiShadows::DrawShadowRect(
                    draw_list, shadow_tex, shadow_uvs, p_min, p_max, col32,
                    m_resolvedShadowThickness, m_resolvedShadowOffset, rounding, m_resolvedShadowFillBackground);
            }

            // 4. Merge
            splitter.Merge(draw_list);
        }

        void onHandleDestroyed() override { ownsHandle = false; }
        void verifyState() const { App::assertMainThread(); }

        template <typename... Args, typename F>
        std::shared_ptr<Derived> observe(Signal<Args...> &signal, F cb)
        {
            m_connections.push_back(signal.connect(std::function<void(Args...)>(std::move(cb))));
            return self();
        }

        std::string getID() const override
        {
            return m_customId;
        }

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

        std::shared_ptr<Derived> setVisible(bool visible)
        {
            this->visible = visible;
            return self();
        }
        bool isVisible() const { return visible; }

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
        std::shared_ptr<Derived> setShadow(bool enable, ImVec2 offset = ImVec2(0, 0), float blur = 10.0f, ImVec4 col = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f), float rounding = -1.0f, float thickness = -1.0f)
        {
            inlineShadowEnabled = enable;
            inlineShadowOffset = offset;
            inlineShadowBlur = blur;
            inlineShadowColor = col;
            inlineShadowRounding = rounding;
            inlineShadowThickness = (thickness < 0.0f) ? blur : thickness; // fallback to blur for backward compatibility
            m_styleVersion = 0;
            return self();
        }

        std::shared_ptr<Derived> setShadowEnabled(bool enable)
        {
            inlineShadowEnabled = enable;
            m_styleVersion = 0;
            return self();
        }

        std::shared_ptr<Derived> setShadowOffset(ImVec2 offset)
        {
            inlineShadowOffset = offset;
            m_styleVersion = 0;
            return self();
        }

        std::shared_ptr<Derived> setShadowBlur(float blur)
        {
            inlineShadowBlur = blur;
            m_styleVersion = 0;
            return self();
        }

        std::shared_ptr<Derived> setShadowColor(ImVec4 col)
        {
            inlineShadowColor = col;
            m_styleVersion = 0;
            return self();
        }

        std::shared_ptr<Derived> setShadowRounding(float rounding)
        {
            inlineShadowRounding = rounding;
            m_styleVersion = 0;
            return self();
        }

        std::shared_ptr<Derived> setShadowFillBackground(bool fill)
        {
            inlineShadowFillBackground = fill;
            m_styleVersion = 0;
            return self();
        }

        std::shared_ptr<Derived> setShadowThickness(float thickness)
        {
            inlineShadowThickness = thickness;
            m_styleVersion = 0;
            return self();
        }

        bool getShadowEnabled() const { return inlineShadowEnabled.value_or(StyleSheet::s_defaultHasShadow); }
        ImVec2 getShadowOffset() const { return inlineShadowOffset.value_or(StyleSheet::s_defaultShadowOffset); }
        float getShadowBlur() const { return inlineShadowBlur.value_or(StyleSheet::s_defaultShadowBlur); }
        ImVec4 getShadowColor() const { return inlineShadowColor.value_or(StyleSheet::s_defaultShadowColor); }
        float getShadowRounding() const { return inlineShadowRounding.value_or(StyleSheet::s_defaultShadowRounding); }
        bool getShadowFillBackground() const { return inlineShadowFillBackground.value_or(StyleSheet::s_defaultShadowFillBackground); }
        float getShadowThickness() const { return inlineShadowThickness.value_or(StyleSheet::s_defaultShadowThickness); }

        std::shared_ptr<Derived> defaultShadow(bool enable = true)
        {
            inlineShadowEnabled = StyleSheet::s_defaultHasShadow || enable;
            inlineShadowOffset = StyleSheet::s_defaultShadowOffset;
            inlineShadowBlur = StyleSheet::s_defaultShadowBlur;
            inlineShadowColor = StyleSheet::s_defaultShadowColor;
            inlineShadowRounding = StyleSheet::s_defaultShadowRounding;
            inlineShadowFillBackground = StyleSheet::s_defaultShadowFillBackground;
            inlineShadowThickness = StyleSheet::s_defaultShadowThickness;
            m_styleVersion = 0;
            return self();
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

    inline ScopedControlID::ScopedControlID(IControl *ctrl)
    {
        if (!ctrl->getID().empty())
        {
            ImGui::PushID(ctrl->getID().c_str());
            pushed = true;
        }
    }
} // namespace mui
