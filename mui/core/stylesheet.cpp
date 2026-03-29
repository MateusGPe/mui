// core/stylesheet.cpp
#include "stylesheet.hpp"
#include <toml++/toml.hpp>
#include <string_view>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <iostream>

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

    using StyleVarMap = std::unordered_map<std::string_view, ImGuiStyleVar, ci_hash, ci_equal>;
    static const StyleVarMap styleVarMap = {
        {"alpha", ImGuiStyleVar_Alpha},
        {"buttontextalign", ImGuiStyleVar_ButtonTextAlign},
        {"cellpadding", ImGuiStyleVar_CellPadding},
        {"childbordersize", ImGuiStyleVar_ChildBorderSize},
        {"childrounding", ImGuiStyleVar_ChildRounding},
        {"disabledalpha", ImGuiStyleVar_DisabledAlpha},
        {"dockingseparatorsize", ImGuiStyleVar_DockingSeparatorSize},
        {"framebordersize", ImGuiStyleVar_FrameBorderSize},
        {"framepadding", ImGuiStyleVar_FramePadding},
        {"framerounding", ImGuiStyleVar_FrameRounding},
        {"grabminsize", ImGuiStyleVar_GrabMinSize},
        {"grabrounding", ImGuiStyleVar_GrabRounding},
        {"imagebordersize", ImGuiStyleVar_ImageBorderSize},
        {"imagerounding", ImGuiStyleVar_ImageRounding},
        {"indentspacing", ImGuiStyleVar_IndentSpacing},
        {"iteminnerspacing", ImGuiStyleVar_ItemInnerSpacing},
        {"itemspacing", ImGuiStyleVar_ItemSpacing},
        {"popupbordersize", ImGuiStyleVar_PopupBorderSize},
        {"popuprounding", ImGuiStyleVar_PopupRounding},
        {"scrollbarpadding", ImGuiStyleVar_ScrollbarPadding},
        {"scrollbarrounding", ImGuiStyleVar_ScrollbarRounding},
        {"scrollbarsize", ImGuiStyleVar_ScrollbarSize},
        {"selectabletextalign", ImGuiStyleVar_SelectableTextAlign},
        {"separatorsize", ImGuiStyleVar_SeparatorSize},
        {"separatortextalign", ImGuiStyleVar_SeparatorTextAlign},
        {"separatortextbordersize", ImGuiStyleVar_SeparatorTextBorderSize},
        {"separatortextpadding", ImGuiStyleVar_SeparatorTextPadding},
        {"tabbarbordersize", ImGuiStyleVar_TabBarBorderSize},
        {"tabbaroverlinesize", ImGuiStyleVar_TabBarOverlineSize},
        {"tabbordersize", ImGuiStyleVar_TabBorderSize},
        {"tabminwidthbase", ImGuiStyleVar_TabMinWidthBase},
        {"tabminwidthshrink", ImGuiStyleVar_TabMinWidthShrink},
        {"tabrounding", ImGuiStyleVar_TabRounding},
        {"tableangledheadersangle", ImGuiStyleVar_TableAngledHeadersAngle},
        {"tableangledheaderstextalign", ImGuiStyleVar_TableAngledHeadersTextAlign},
        {"treelinesrounding", ImGuiStyleVar_TreeLinesRounding},
        {"treelinessize", ImGuiStyleVar_TreeLinesSize},
        {"windowbordersize", ImGuiStyleVar_WindowBorderSize},
        {"windowminsize", ImGuiStyleVar_WindowMinSize},
        {"windowpadding", ImGuiStyleVar_WindowPadding},
        {"windowrounding", ImGuiStyleVar_WindowRounding},
        {"windowtitlealign", ImGuiStyleVar_WindowTitleAlign},
    };

    using ColorMap = std::unordered_map<std::string_view, ImGuiCol, ci_hash, ci_equal>;
    static const ColorMap colorMap = {
        {"border", ImGuiCol_Border},
        {"bordershadow", ImGuiCol_BorderShadow},
        {"button", ImGuiCol_Button},
        {"buttonactive", ImGuiCol_ButtonActive},
        {"buttonhovered", ImGuiCol_ButtonHovered},
        {"checkmark", ImGuiCol_CheckMark},
        {"childbg", ImGuiCol_ChildBg},
        {"dockingemptybg", ImGuiCol_DockingEmptyBg},
        {"dockingpreview", ImGuiCol_DockingPreview},
        {"dragdroptarget", ImGuiCol_DragDropTarget},
        {"dragdroptargetbg", ImGuiCol_DragDropTargetBg},
        {"framebg", ImGuiCol_FrameBg},
        {"framebgactive", ImGuiCol_FrameBgActive},
        {"framebghovered", ImGuiCol_FrameBgHovered},
        {"header", ImGuiCol_Header},
        {"headeractive", ImGuiCol_HeaderActive},
        {"headerhovered", ImGuiCol_HeaderHovered},
        {"inputtextcursor", ImGuiCol_InputTextCursor},
        {"menubarbg", ImGuiCol_MenuBarBg},
        {"modalwindowdimbg", ImGuiCol_ModalWindowDimBg},
        {"navcursor", ImGuiCol_NavCursor},
        {"navhighlight", ImGuiCol_NavHighlight},
        {"navwindowingdimbg", ImGuiCol_NavWindowingDimBg},
        {"navwindowinghighlight", ImGuiCol_NavWindowingHighlight},
        {"plothistogram", ImGuiCol_PlotHistogram},
        {"plothistogramhovered", ImGuiCol_PlotHistogramHovered},
        {"plotlines", ImGuiCol_PlotLines},
        {"plotlineshovered", ImGuiCol_PlotLinesHovered},
        {"popupbg", ImGuiCol_PopupBg},
        {"resizegrip", ImGuiCol_ResizeGrip},
        {"resizegripactive", ImGuiCol_ResizeGripActive},
        {"resizegriphovered", ImGuiCol_ResizeGripHovered},
        {"scrollbarbg", ImGuiCol_ScrollbarBg},
        {"scrollbargrab", ImGuiCol_ScrollbarGrab},
        {"scrollbargrabactive", ImGuiCol_ScrollbarGrabActive},
        {"scrollbargrabhovered", ImGuiCol_ScrollbarGrabHovered},
        {"separator", ImGuiCol_Separator},
        {"separatoractive", ImGuiCol_SeparatorActive},
        {"separatorhovered", ImGuiCol_SeparatorHovered},
        {"slidergrab", ImGuiCol_SliderGrab},
        {"slidergrabactive", ImGuiCol_SliderGrabActive},
        {"tab", ImGuiCol_Tab},
        {"tabactive", ImGuiCol_TabActive},
        {"tabdimmed", ImGuiCol_TabDimmed},
        {"tabdimmedselected", ImGuiCol_TabDimmedSelected},
        {"tabdimmedselectedoverline", ImGuiCol_TabDimmedSelectedOverline},
        {"tabhovered", ImGuiCol_TabHovered},
        {"tableborderlight", ImGuiCol_TableBorderLight},
        {"tableborderstrong", ImGuiCol_TableBorderStrong},
        {"tableheaderbg", ImGuiCol_TableHeaderBg},
        {"tablerowbg", ImGuiCol_TableRowBg},
        {"tablerowbgalt", ImGuiCol_TableRowBgAlt},
        {"tabselected", ImGuiCol_TabSelected},
        {"tabselectedoverline", ImGuiCol_TabSelectedOverline},
        {"tabunfocused", ImGuiCol_TabUnfocused},
        {"tabunfocusedactive", ImGuiCol_TabUnfocusedActive},
        {"text", ImGuiCol_Text},
        {"textdisabled", ImGuiCol_TextDisabled},
        {"textlink", ImGuiCol_TextLink},
        {"textselectedbg", ImGuiCol_TextSelectedBg},
        {"titlebg", ImGuiCol_TitleBg},
        {"titlebgactive", ImGuiCol_TitleBgActive},
        {"titlebgcollapsed", ImGuiCol_TitleBgCollapsed},
        {"treelines", ImGuiCol_TreeLines},
        {"unsavedmarker", ImGuiCol_UnsavedMarker},
        {"windowbg", ImGuiCol_WindowBg},
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

    bool StyleSheet::loadFromFile(const std::string &filepath)
    {
        try
        {
            toml::table tbl = toml::parse_file(filepath);

            for (const auto &[selector_key, selector_node] : tbl)
            {
                if (!selector_node.is_table())
                    continue;

                std::string selector_str = std::string(selector_key.str());
                StyleBlock &block = StyleSheet::select(selector_str); // This also increments version

                const toml::table &properties = *selector_node.as_table();

                for (const auto &[prop_key, prop_node] : properties)
                {
                    std::string_view key_sv = prop_key.str();

                    // Check for colors
                    if (auto it = colorMap.find(key_sv); it != colorMap.end())
                    {
                        if (auto color = parse_color(prop_node))
                        {
                            block.color(it->second, *color);
                        }
                        continue;
                    }

                    // Check for style vars
                    if (auto it = styleVarMap.find(key_sv); it != styleVarMap.end())
                    {
                        if (prop_node.is_floating_point() || prop_node.is_integer())
                        {
                            block.var(it->second, prop_node.value_or<float>(0.0f));
                        }
                        else if (auto vec2 = parse_vec2(prop_node))
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
                        if (auto vec2 = parse_vec2(prop_node))
                            block.shadowOffset = *vec2;
                    }
                    else if (key_sv == "shadowBlur")
                    {
                        block.shadowBlur = prop_node.value<float>();
                    }
                    else if (key_sv == "shadowColor")
                    {
                        if (auto color = parse_color(prop_node))
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
        catch (const toml::parse_error &err)
        {
            std::cerr << "Error parsing stylesheet TOML file '" << filepath << "':\n"
                      << err << std::endl;
            return false;
        }
        return true;
    }

} // namespace mui