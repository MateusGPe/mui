// core/stylesheet.hpp
#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <optional>

namespace mui
{
    struct StyleBlock
    {
        std::vector<std::pair<ImGuiCol, ImVec4>> colors;
        std::vector<std::pair<ImGuiStyleVar, float>> floats;
        std::vector<std::pair<ImGuiStyleVar, ImVec2>> vec2s;

        // Optional shadow overrides for this style block
        std::optional<bool> shadowEnabled;
        std::optional<ImVec2> shadowOffset;
        std::optional<float> shadowBlur;
        std::optional<ImVec4> shadowColor;
        std::optional<float> shadowRounding;
        std::optional<bool> shadowFillBackground;
        std::optional<float> shadowThickness;

        StyleBlock &color(ImGuiCol col, ImVec4 val)
        {
            colors.push_back({col, val});
            return *this;
        }
        StyleBlock &var(ImGuiStyleVar var, float val)
        {
            floats.push_back({var, val});
            return *this;
        }
        StyleBlock &var(ImGuiStyleVar var, ImVec2 val)
        {
            vec2s.push_back({var, val});
            return *this;
        }

        StyleBlock &shadow(bool enable, ImVec2 offset = ImVec2(0, 0), float blur = 10.0f, ImVec4 col = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f), float rounding = -1.0f, float thickness = -1.0f)
        {
            shadowEnabled = enable;
            shadowOffset = offset;
            shadowBlur = blur;
            shadowColor = col;
            shadowRounding = rounding;
            shadowThickness = (thickness < 0.0f) ? blur : thickness;
            return *this;
        }

        StyleBlock &setShadowEnabled(bool enable)
        {
            shadowEnabled = enable;
            return *this;
        }
        StyleBlock &setShadowOffset(ImVec2 offset)
        {
            shadowOffset = offset;
            return *this;
        }
        StyleBlock &setShadowBlur(float blur)
        {
            shadowBlur = blur;
            return *this;
        }
        StyleBlock &setShadowColor(ImVec4 col)
        {
            shadowColor = col;
            return *this;
        }
        StyleBlock &setShadowRounding(float rounding)
        {
            shadowRounding = rounding;
            return *this;
        }
        StyleBlock &setShadowFillBackground(bool fill)
        {
            shadowFillBackground = fill;
            return *this;
        }
        StyleBlock &setShadowThickness(float thickness)
        {
            shadowThickness = thickness;
            return *this;
        }
    };

    class StyleSheet
    {
    private:
        static inline std::unordered_map<std::string, StyleBlock> m_styles;
        static inline unsigned int m_version = 1;

    public:
        // Global Shadow Defaults
        static inline bool s_defaultHasShadow = false;
        static inline ImVec2 s_defaultShadowOffset = ImVec2(0.0f, 0.0f);
        static inline float s_defaultShadowBlur = 8.0f;
        static inline ImVec4 s_defaultShadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
        static inline float s_defaultShadowRounding = 8.0f;
        static inline bool s_defaultShadowFillBackground = true;
        static inline float s_defaultShadowThickness = -1.0f;

        static void setGlobalShadowDefaults(bool enable, ImVec2 offset = ImVec2(0, 0), float blur = 10.0f, ImVec4 col = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f), float rounding = -1.0f, float thickness = -1.0f)
        {
            s_defaultHasShadow = enable;
            s_defaultShadowOffset = offset;
            s_defaultShadowBlur = blur;
            s_defaultShadowColor = col;
            s_defaultShadowRounding = rounding;
            s_defaultShadowThickness = (thickness < 0.0f) ? blur : thickness;
            m_version++;
        }

        static void setGlobalShadowEnabled(bool enable)
        {
            s_defaultHasShadow = enable;
            m_version++;
        }
        static void setGlobalShadowOffset(ImVec2 offset)
        {
            s_defaultShadowOffset = offset;
            m_version++;
        }
        static void setGlobalShadowBlur(float blur)
        {
            s_defaultShadowBlur = blur;
            m_version++;
        }
        static void setGlobalShadowColor(ImVec4 col)
        {
            s_defaultShadowColor = col;
            m_version++;
        }
        static void setGlobalShadowRounding(float rounding)
        {
            s_defaultShadowRounding = rounding;
            m_version++;
        }
        static void setGlobalShadowFillBackground(bool fill)
        {
            s_defaultShadowFillBackground = fill;
            m_version++;
        }
        static void setGlobalShadowThickness(float thickness)
        {
            s_defaultShadowThickness = thickness;
            m_version++;
        }

        static bool getGlobalShadowEnabled() { return s_defaultHasShadow; }
        static ImVec2 getGlobalShadowOffset() { return s_defaultShadowOffset; }
        static float getGlobalShadowBlur() { return s_defaultShadowBlur; }
        static ImVec4 getGlobalShadowColor() { return s_defaultShadowColor; }
        static float getGlobalShadowRounding() { return s_defaultShadowRounding; }
        static bool getGlobalShadowFillBackground() { return s_defaultShadowFillBackground; }
        static float getGlobalShadowThickness() { return s_defaultShadowThickness; }

        // Define a CSS-like selector: "Button" (Type), ".danger" (Class), "#my_id" (ID)
        static StyleBlock &select(const std::string &selector)
        {
            m_version++; // Invalidate widget caches
            return m_styles[selector];
        }

        static const StyleBlock *get(const std::string &selector)
        {
            auto it = m_styles.find(selector);
            return it != m_styles.end() ? &it->second : nullptr;
        }

        static unsigned int getVersion() { return m_version; }
    };
}