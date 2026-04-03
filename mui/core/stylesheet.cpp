// core/stylesheet.cpp
#include "stylesheet.hpp"
#include "style_parser_utils.hpp" // Shared parsing utilities
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string_view>
#include <unordered_map>

namespace mui
{
    // CRITICAL - Override Behavior: This method now parses a TOML table directly,
    // allowing it to be called from Theme::loadThemeFromToml for baseline styles,
    // or from StyleSheet::loadFromFile for overriding/appending.
    bool StyleSheet::loadFromNode(const toml::table &tbl)
    {
        try
        {
            for (const auto &[selector_key, selector_node] : tbl)
            {
                if (!selector_node.is_table())
                    continue;

                std::string selector_str = std::string(selector_key.str());
                StyleBlock &block =
                    StyleSheet::select(selector_str); // This also increments version

                const toml::table &properties = *selector_node.as_table();

                for (const auto &[prop_key, prop_node] : properties)
                {
                    std::string_view key_sv = prop_key.str();

                    // Check for colors
                    if (auto it = mui::colorMap.find(key_sv); it != mui::colorMap.end())
                    {
                        if (auto color = mui::parse_color(prop_node))
                        {
                            block.color(it->second, *color);
                        }
                        continue;
                    }

                    // Check for style vars
                    if (auto it = mui::styleVarMap.find(key_sv);
                        it != mui::styleVarMap.end())
                    {
                        if (prop_node.is_floating_point() || prop_node.is_integer())
                        {
                            block.var(it->second, prop_node.value_or<float>(
                                                      0.0f)); // Use value_or for safety
                        }
                        else if (auto vec2 = mui::parse_vec2(prop_node))
                        {
                            block.var(it->second, *vec2);
                        }
                        continue;
                    }

                    // Check for shadow properties
                    if (key_sv == "shadowEnabled")
                    {
                        block.shadowEnabled = prop_node.value<bool>();
                    }
                    else if (key_sv == "shadowOffset")
                    {
                        if (auto vec2 = mui::parse_vec2(prop_node))
                            block.shadowOffset = *vec2;
                    }
                    else if (key_sv == "shadowBlur")
                    {
                        block.shadowBlur = prop_node.value<float>();
                    }
                    else if (key_sv == "shadowColor")
                    {
                        if (auto color = mui::parse_color(prop_node))
                            block.shadowColor = *color;
                    }
                    else if (key_sv == "shadowRounding")
                    {
                        block.shadowRounding = prop_node.value<float>();
                    }
                    else if (key_sv == "shadowFillBackground")
                    {
                        block.shadowFillBackground = prop_node.value<bool>();
                    }
                    else if (key_sv == "shadowThickness")
                    {
                        block.shadowThickness = prop_node.value<float>();
                    }
                }
            }
        }
        catch (const toml::parse_error &)
        {
            // Errors are handled by the caller (Theme::loadThemeFromToml)
            return false;
        }
        return true;
    }

    bool StyleSheet::loadFromFile(const std::string &filepath)
    {
        try
        {
            toml::table tbl = toml::parse_file(filepath);
            return loadFromNode(tbl); // Call the new shared parsing logic
        }
        catch (const toml::parse_error &err)
        {
            std::cerr << "Error parsing stylesheet TOML file '" << filepath << "':\n"
                      << err << std::endl;
            return false;
        }
    }

} // namespace mui