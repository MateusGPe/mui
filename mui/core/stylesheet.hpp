// core/stylesheet.hpp
#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

namespace mui
{
    struct StyleBlock
    {
        std::vector<std::pair<ImGuiCol, ImVec4>> colors;
        std::vector<std::pair<ImGuiStyleVar, float>> floats;
        std::vector<std::pair<ImGuiStyleVar, ImVec2>> vec2s;

        StyleBlock& color(ImGuiCol col, ImVec4 val) { colors.push_back({col, val}); return *this; }
        StyleBlock& var(ImGuiStyleVar var, float val) { floats.push_back({var, val}); return *this; }
        StyleBlock& var(ImGuiStyleVar var, ImVec2 val) { vec2s.push_back({var, val}); return *this; }
    };

    class StyleSheet
    {
    private:
        static inline std::unordered_map<std::string, StyleBlock> m_styles;
        static inline unsigned int m_version = 1;

    public:
        // Define a CSS-like selector: "Button" (Type), ".danger" (Class), "#my_id" (ID)
        static StyleBlock& select(const std::string& selector)
        {
            m_version++; // Invalidate widget caches
            return m_styles[selector];
        }

        static const StyleBlock* get(const std::string& selector)
        {
            auto it = m_styles.find(selector);
            return it != m_styles.end() ? &it->second : nullptr;
        }

        static unsigned int getVersion() { return m_version; }
    };
}