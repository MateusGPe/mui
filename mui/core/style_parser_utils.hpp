#pragma once
#include <toml++/toml.hpp>
#include <string_view>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <optional>
#include <imgui.h> // For ImGuiCol, ImGuiStyleVar, ImVec4, ImVec2

namespace mui
{
    // Case-insensitive string comparison for map keys
    struct ci_equal
    {
        bool operator()(std::string_view s1, std::string_view s2) const
        {
            return std::equal(
                s1.begin(), s1.end(), s2.begin(), s2.end(),
                [](char a, char b)
                {
                    return std::tolower(static_cast<unsigned char>(a)) ==
                           std::tolower(static_cast<unsigned char>(b));
                });
        }
    };

    struct ci_hash
    {
        // FNV-1a constants for 64-bit hash
        static constexpr size_t FNV_prime = 1099511628211ULL;
        static constexpr size_t FNV_offset_basis = 14695981039346656037ULL;

        size_t operator()(std::string_view s) const
        {
            size_t h = FNV_offset_basis;
            for (char c : s)
            {
                h ^= std::tolower(static_cast<unsigned char>(c));
                h *= FNV_prime;
            }
            return h;
        }
    };
    // Helper to parse a color from a TOML node (array or hex string)
    static std::optional<ImVec4> parse_color(const toml::node &node)
    {
        if (auto arr = node.as_array())
        {
            if (arr->size() >= 3)
            {
                float r = arr->get(0)->value_or<float>(0.0f);
                float g = arr->get(1)->value_or<float>(0.0f);
                float b = arr->get(2)->value_or<float>(0.0f);
                float a = arr->size() >= 4 ? arr->get(3)->value_or<float>(1.0f) : 1.0f;
                return ImVec4(r, g, b, a);
            }
        }
        else if (auto str_node = node.as_string())
        {
            std::string hex = str_node->get();
            if (hex.length() >= 7 && hex[0] == '#')
            {
                unsigned int r = 0, g = 0, b = 0, a = 255;
                if (hex.length() >= 9)
                {
                    sscanf(hex.c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);
                }
                else
                {
                    sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
                }
                return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
            }
        }
        return std::nullopt;
    }

    // Helper to parse an ImVec2 from a TOML node
    static std::optional<ImVec2> parse_vec2(const toml::node &node)
    {
        if (auto arr = node.as_array())
        {
            if (arr->size() >= 2)
            {
                float x = arr->get(0)->value_or<float>(0.0f);
                float y = arr->get(1)->value_or<float>(0.0f);
                return ImVec2(x, y);
            }
        }
        return std::nullopt;
    }


    // Shared mappings for ImGuiCol and ImGuiStyleVar
    extern const std::unordered_map<std::string_view, ImGuiCol, ci_hash, ci_equal> colorMap;
    extern const std::unordered_map<std::string_view, ImGuiStyleVar, ci_hash, ci_equal> styleVarMap;

} // namespace mui