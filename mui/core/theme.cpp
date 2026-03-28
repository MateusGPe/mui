#include "theme.hpp"
#include <imgui.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <unordered_map>
#include <cmath>
#include <toml++/toml.hpp>
#include <algorithm>
#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h> // For SHGetFolderPathA
#elif defined(__linux__)
#include <fontconfig/fontconfig.h>
#endif

#include "../include/IconsFontAwesome6.h"
#include "fa_solid_900.hpp"
#include <imgui_internal.h>

namespace mui
{
    // Helper to verify file existence before passing to ImGui
    inline bool fileExists(const std::string &name)
    {
        std::ifstream f(name.c_str());
        return f.good();
    }

    void Theme::loadSystemFont(float fontSize)
    {
        ImGuiIO &io = ImGui::GetIO();
        std::string fontPath = "";

#if defined(_WIN32)
        CHAR szPath[MAX_PATH];
        // Get the path to the Windows fonts directory, which is more robust than a hardcoded path.
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, szPath)))
        {
            fontPath = std::string(szPath) + "\\segoeui.ttf";
        }
        // Fallback to the hardcoded path if the above fails
        if (fontPath.empty() || !fileExists(fontPath))
        {
            fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
        }
#elif defined(__APPLE__)
        // The system font path on macOS is very stable.
        fontPath = "/System/Library/Fonts/Supplemental/Helvetica.ttc";
#elif defined(__linux__)
        { // The ideal way on Linux is to use fontconfig to find a default font.
            // This requires linking against the fontconfig library.
            FcConfig *config = FcInitLoadConfigAndFonts();
            if (config)
            {
                FcPattern *pat = FcNameParse((const FcChar8 *)"sans-serif");
                FcConfigSubstitute(config, pat, FcMatchPattern);
                FcDefaultSubstitute(pat);

                FcResult result;
                FcPattern *font = FcFontMatch(config, pat, &result);

                if (font)
                {
                    FcChar8 *file = NULL;
                    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
                    {
                        fontPath = (const char *)file;
                    }
                    FcPatternDestroy(font);
                }
                FcPatternDestroy(pat);
                FcConfigDestroy(config);
            }

            // Fallback to searching known paths if fontconfig fails or is not available
            if (fontPath.empty() || !fileExists(fontPath))
            {
                const char *linuxFonts[] = {
                    "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
                    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                    "/usr/share/fonts/TTF/DejaVuSans.ttf",
                    "/usr/share/fonts/noto/NotoSans-Regular.ttf" // Add Noto as another common option
                };
                for (const char *path : linuxFonts)
                {
                    if (fileExists(path))
                    {
                        fontPath = path;
                        break;
                    }
                }
            }
        }
#endif

        // 1. Configure and Load the Base Text Font
        ImFontConfig config;
        config.OversampleH = 3;
        config.OversampleV = 3;
        config.PixelSnapH = false;

        if (!fontPath.empty() && fileExists(fontPath))
        {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, &config);
        }
        else
        {
            io.Fonts->AddFontDefault(&config);
        }

        // 2. Configure and Merge the Icon Font
        ImFontConfig icon_config;
        icon_config.MergeMode = true;            // CRITICAL: Merges with the previously loaded font
        icon_config.PixelSnapH = true;           // Align icons to pixels for sharpness
        icon_config.GlyphMinAdvanceX = fontSize; // Forces icons to be monospaced
        icon_config.OversampleH = 3;
        icon_config.OversampleV = 3;

        // Define the Unicode range for FontAwesome 6
        static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};

        // Load the embedded icon font from memory
        io.Fonts->AddFontFromMemoryCompressedTTF(
            fa_solid_900_compressed_data,
            fa_solid_900_compressed_size,
            fontSize,
            &icon_config,
            icon_ranges);
    }

    void Theme::applyStyle(float dpiScale)
    {
        ImGuiStyle &style = ImGui::GetStyle();

        // --- ENFORCE ANTI-ALIASING ---
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        // This flag uses a texture-based method for anti-aliasing lines. While it can produce
        // smoother lines, it has been known to cause rendering artifacts with other elements,
        // such as the 1-pixel-wide text input cursor, causing it to appear white.
        style.AntiAliasedLinesUseTex = true;

        style.WindowRounding = 8.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        // Erase Borders
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 0.0f;
        style.PopupBorderSize = 0.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;

        // Spacing & Padding
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.FramePadding = ImVec2(8.0f, 6.0f);
        style.WindowPadding = ImVec2(8.0f, 8.0f);

        // --- APPLY DPI SCALE ---
        style.ScaleAllSizes(dpiScale);
        // 4. Color Palette (Light Material Design)
        ImVec4 *colors = style.Colors;
        colors[ImGuiCol_Text] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Pure white cards
        colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
        colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.08f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.80f, 0.80f, 0.85f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
        colors[ImGuiCol_NavCursor] = ImVec4(0.12f, 0.53f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.95f, 0.95f, 0.97f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.43f, 0.76f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f); // Flutter primary blue
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.43f, 0.76f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.12f, 0.53f, 0.90f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.12f, 0.53f, 0.90f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.53f, 0.90f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_InputTextCursor] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);

        // --- Add missing colors for a more complete theme ---
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // Obsolete
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.12f, 0.53f, 0.90f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.95f, 0.95f, 0.97f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.12f, 0.53f, 0.90f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.12f, 0.53f, 0.90f, 0.95f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.12f, 0.53f, 0.90f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = colors[ImGuiCol_FrameBg];
        colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = colors[ImGuiCol_Button];
        colors[ImGuiCol_PlotHistogramHovered] = colors[ImGuiCol_ButtonHovered];
        colors[ImGuiCol_TableHeaderBg] = colors[ImGuiCol_FrameBg];
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.70f, 0.70f, 0.75f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.80f, 0.80f, 0.85f, 1.00f);
        colors[ImGuiCol_TableRowBg] = colors[ImGuiCol_ChildBg];
        colors[ImGuiCol_TableRowBgAlt] = colors[ImGuiCol_WindowBg];
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.12f, 0.53f, 0.90f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }

    void Theme::applyDarkStyle(float dpiScale)
    {
        ImGuiStyle &style = ImGui::GetStyle();

        // --- ENFORCE ANTI-ALIASING ---
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        style.AntiAliasedLinesUseTex = true;

        // Geometry & Rounding
        style.WindowRounding = 8.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        // Borders
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;

        // Spacing & Padding
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.FramePadding = ImVec2(8.0f, 6.0f);
        style.WindowPadding = ImVec2(8.0f, 8.0f);

        // --- APPLY DPI SCALE ---
        style.ScaleAllSizes(dpiScale);

        // 4. Color Palette (Modern Dark Theme)
        ImVec4 *colors = style.Colors;

        ImVec4 bg_main = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);
        ImVec4 bg_child = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
        ImVec4 bg_frame = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);
        ImVec4 accent = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        ImVec4 accent_hover = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
        ImVec4 accent_active = ImVec4(0.20f, 0.50f, 0.90f, 1.00f);
        ImVec4 text_main = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        ImVec4 border_color = ImVec4(0.25f, 0.26f, 0.28f, 1.00f);

        colors[ImGuiCol_Text] = text_main;
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg] = bg_main;
        colors[ImGuiCol_ChildBg] = bg_child;
        colors[ImGuiCol_PopupBg] = bg_main;
        colors[ImGuiCol_Border] = border_color;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = bg_frame;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.26f, 0.28f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.31f, 0.33f, 1.00f);
        colors[ImGuiCol_TitleBg] = bg_child;
        colors[ImGuiCol_TitleBgActive] = bg_child;
        colors[ImGuiCol_TitleBgCollapsed] = bg_child;
        colors[ImGuiCol_MenuBarBg] = bg_child;
        colors[ImGuiCol_ScrollbarBg] = bg_frame;
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accent_active;
        colors[ImGuiCol_Button] = accent;
        colors[ImGuiCol_ButtonHovered] = accent_hover;
        colors[ImGuiCol_ButtonActive] = accent_active;
        colors[ImGuiCol_Header] = accent;
        colors[ImGuiCol_HeaderHovered] = accent_hover;
        colors[ImGuiCol_HeaderActive] = accent_active;
        colors[ImGuiCol_Separator] = border_color;
        colors[ImGuiCol_SeparatorHovered] = accent_hover;
        colors[ImGuiCol_SeparatorActive] = accent;
        colors[ImGuiCol_ResizeGrip] = bg_frame;
        colors[ImGuiCol_ResizeGripHovered] = accent_hover;
        colors[ImGuiCol_ResizeGripActive] = accent;
        colors[ImGuiCol_Tab] = bg_child;
        colors[ImGuiCol_TabHovered] = accent_hover;
        colors[ImGuiCol_TabActive] = accent;
        colors[ImGuiCol_TabUnfocused] = bg_main;
        colors[ImGuiCol_TabUnfocusedActive] = bg_child;
        colors[ImGuiCol_DockingPreview] = accent;
        colors[ImGuiCol_DockingEmptyBg] = bg_main;
        colors[ImGuiCol_PlotLines] = accent;
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = accent;
        colors[ImGuiCol_PlotHistogramHovered] = accent_hover;
        colors[ImGuiCol_TableHeaderBg] = bg_child;
        colors[ImGuiCol_TableBorderStrong] = border_color;
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_TableRowBg] = bg_main;
        colors[ImGuiCol_TableRowBgAlt] = bg_child;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = accent;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.35f);
    }

    std::vector<std::string> Theme::getAvailableThemes(const std::string& filepath)
    {
        std::vector<std::string> themes;
        try
        {
            toml::table config = toml::parse_file(filepath);

            // Helper to find a node by its key case-insensitively
            auto findCaseInsensitive = [](const toml::table* table, std::string_view searchKey) -> const toml::node* {
                if (!table) return nullptr;
                for (const auto& [key, node] : *table) {
                    if (key.str().length() == searchKey.length()) {
                        auto kStr = key.str();
                        if (std::equal(kStr.begin(), kStr.end(), searchKey.begin(), [](char a, char b) {
                            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
                        })) {
                            return &node;
                        }
                    }
                }
                return nullptr;
            };

            auto themesNodeRaw = findCaseInsensitive(&config, "themes");
            if (themesNodeRaw && themesNodeRaw->as_table())
            {
                for (const auto& [key, _] : *themesNodeRaw->as_table())
                {
                    themes.push_back(std::string(key.str()));
                }
            }
        }
        catch (const toml::parse_error&) {}
        return themes;
    }

    bool Theme::loadThemeFromToml(const std::string &filepath, const std::string &themeName, float dpiScale)
    {
        applyDarkStyle(dpiScale); // Start with a base style to ensure all properties are initialized
        try
        {
            toml::table config = toml::parse_file(filepath);

            // Helper to find a node by its key case-insensitively
            auto findCaseInsensitive = [](const toml::table* table, std::string_view searchKey) -> const toml::node* {
                if (!table) return nullptr;
                for (const auto& [key, node] : *table) {
                    if (key.str().length() == searchKey.length()) {
                        auto kStr = key.str();
                        if (std::equal(kStr.begin(), kStr.end(), searchKey.begin(), [](char a, char b) {
                            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
                        })) {
                            return &node;
                        }
                    }
                }
                return nullptr;
            };

            // Navigate to [themes.ThemeName]
            auto themesNodeRaw = findCaseInsensitive(&config, "themes");
            if (!themesNodeRaw || !themesNodeRaw->as_table()) return false;
            
            auto themeNodeRaw = findCaseInsensitive(themesNodeRaw->as_table(), themeName);
            if (!themeNodeRaw || !themeNodeRaw->as_table()) return false;
            
            auto themeTable = themeNodeRaw->as_table();

            ImGuiStyle &style = ImGui::GetStyle();

            // Parse style geometry and properties
            if (auto styleNodeRaw = findCaseInsensitive(themeTable, "style"))
            {
                if (auto styleTable = styleNodeRaw->as_table())
                {
                    if (auto node = findCaseInsensitive(styleTable, "AntiAliasedLines"))
                        if (auto val = node->value<bool>()) style.AntiAliasedLines = *val;
                    if (auto node = findCaseInsensitive(styleTable, "AntiAliasedFill"))
                        if (auto val = node->value<bool>()) style.AntiAliasedFill = *val;
                    if (auto node = findCaseInsensitive(styleTable, "AntiAliasedLinesUseTex"))
                        if (auto val = node->value<bool>()) style.AntiAliasedLinesUseTex = *val;

                    auto getFloat = [&](const char *key, float &out)
                    {
                        if (auto node = findCaseInsensitive(styleTable, key))
                            if (auto val = node->value<float>())
                                out = *val;
                    };

                    getFloat("Alpha", style.Alpha);
                    getFloat("DisabledAlpha", style.DisabledAlpha);
                    getFloat("WindowRounding", style.WindowRounding);
                    getFloat("WindowBorderSize", style.WindowBorderSize);
                    getFloat("ChildRounding", style.ChildRounding);
                    getFloat("ChildBorderSize", style.ChildBorderSize);
                    getFloat("FrameRounding", style.FrameRounding);
                    getFloat("FrameBorderSize", style.FrameBorderSize);
                    getFloat("PopupRounding", style.PopupRounding);
                    getFloat("PopupBorderSize", style.PopupBorderSize);
                    getFloat("IndentSpacing", style.IndentSpacing);
                    getFloat("ColumnsMinSpacing", style.ColumnsMinSpacing);
                    getFloat("ScrollbarSize", style.ScrollbarSize);
                    getFloat("ScrollbarRounding", style.ScrollbarRounding);
                    getFloat("GrabMinSize", style.GrabMinSize);
                    getFloat("GrabRounding", style.GrabRounding);
                    getFloat("LogSliderDeadzone", style.LogSliderDeadzone);
                    getFloat("TabRounding", style.TabRounding);
                    getFloat("TabBorderSize", style.TabBorderSize);
                    getFloat("TabBarBorderSize", style.TabBarBorderSize);
                    getFloat("SeparatorTextBorderSize", style.SeparatorTextBorderSize);
                    getFloat("DockingSeparatorSize", style.DockingSeparatorSize);
                    getFloat("MouseCursorScale", style.MouseCursorScale);
                    getFloat("CurveTessellationTol", style.CurveTessellationTol);
                    getFloat("CircleTessellationMaxError", style.CircleTessellationMaxError);
                    getFloat("HoverStationaryDelay", style.HoverStationaryDelay);
                    getFloat("HoverDelayShort", style.HoverDelayShort);
                    getFloat("HoverDelayNormal", style.HoverDelayNormal);

                    auto getVec2 = [&](const char *key, ImVec2 &out)
                    {
                        if (auto node = findCaseInsensitive(styleTable, key))
                        {
                            if (auto arr = node->as_array())
                            {
                                if (arr->size() >= 2)
                                {
                                    out.x = (*arr)[0].value_or<float>(float(out.x));
                                    out.y = (*arr)[1].value_or<float>(float(out.y));
                                }
                            }
                        }
                    };

                    getVec2("WindowPadding", style.WindowPadding);
                    getVec2("WindowMinSize", style.WindowMinSize);
                    getVec2("WindowTitleAlign", style.WindowTitleAlign);
                    getVec2("FramePadding", style.FramePadding);
                    getVec2("ItemSpacing", style.ItemSpacing);
                    getVec2("ItemInnerSpacing", style.ItemInnerSpacing);
                    getVec2("CellPadding", style.CellPadding);
                    getVec2("TouchExtraPadding", style.TouchExtraPadding);
                    getVec2("ButtonTextAlign", style.ButtonTextAlign);
                    getVec2("SelectableTextAlign", style.SelectableTextAlign);
                    getVec2("SeparatorTextAlign", style.SeparatorTextAlign);
                    getVec2("SeparatorTextPadding", style.SeparatorTextPadding);
                    getVec2("DisplayWindowPadding", style.DisplayWindowPadding);
                    getVec2("DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);

                    auto getDir = [&](const char *key, ImGuiDir &out)
                    {
                        if (auto node = findCaseInsensitive(styleTable, key))
                        {
                            if (auto optVal = node->value<std::string>())
                            {
                                std::string valStr = *optVal;
                                std::transform(valStr.begin(), valStr.end(), valStr.begin(), ::tolower);
                                if (valStr.compare("none") == 0)
                                    out = ImGuiDir_None;
                                else if (valStr.compare("left") == 0)
                                    out = ImGuiDir_Left;
                                else if (valStr.compare("right") == 0)
                                    out = ImGuiDir_Right;
                                else if (valStr.compare("up") == 0)
                                    out = ImGuiDir_Up;
                                else if (valStr.compare("down") == 0)
                                    out = ImGuiDir_Down;
                            }
                        }
                    };
                    getDir("WindowMenuButtonPosition", style.WindowMenuButtonPosition);
                    getDir("ColorButtonPosition", style.ColorButtonPosition);
                    if(style.ColorButtonPosition != ImGuiDir_Left && style.ColorButtonPosition != ImGuiDir_Right)
                        style.ColorButtonPosition = ImGuiDir_Right; // Enforce valid values

                    auto getTreeNodeFlags = [&](const char *key, ImGuiTreeNodeFlags &out)
                    {
                        if (auto node = findCaseInsensitive(styleTable, key))
                        {
                            if (auto arr = node->as_array())
                            {
                                static const std::unordered_map<std::string_view, ImGuiTreeNodeFlags> flagMap = {
                                    {"none", ImGuiTreeNodeFlags_None},
                                    {"selected", ImGuiTreeNodeFlags_Selected},
                                    {"framed", ImGuiTreeNodeFlags_Framed},
                                    {"allowoverlap", ImGuiTreeNodeFlags_AllowOverlap},
                                    {"notreepushonopen", ImGuiTreeNodeFlags_NoTreePushOnOpen},
                                    {"noautoopenonlog", ImGuiTreeNodeFlags_NoAutoOpenOnLog},
                                    {"defaultopen", ImGuiTreeNodeFlags_DefaultOpen},
                                    {"openondoubleclick", ImGuiTreeNodeFlags_OpenOnDoubleClick},
                                    {"openonarrow", ImGuiTreeNodeFlags_OpenOnArrow},
                                    {"leaf", ImGuiTreeNodeFlags_Leaf},
                                    {"bullet", ImGuiTreeNodeFlags_Bullet},
                                    {"framepadding", ImGuiTreeNodeFlags_FramePadding},
                                    {"spanavailwidth", ImGuiTreeNodeFlags_SpanAvailWidth},
                                    {"spanfullwidth", ImGuiTreeNodeFlags_SpanFullWidth},
                                    {"spanlabelwidth", ImGuiTreeNodeFlags_SpanLabelWidth},
                                    {"spanallcolumns", ImGuiTreeNodeFlags_SpanAllColumns},
                                    {"labelspanallcolumns", ImGuiTreeNodeFlags_LabelSpanAllColumns},
                                    {"navleftjumpstoparent", ImGuiTreeNodeFlags_NavLeftJumpsToParent},
                                    {"collapsingheader", ImGuiTreeNodeFlags_CollapsingHeader},
                                    {"drawlinesnone", ImGuiTreeNodeFlags_DrawLinesNone},
                                    {"drawlinesfull", ImGuiTreeNodeFlags_DrawLinesFull},
                                    {"drawlinestonodes", ImGuiTreeNodeFlags_DrawLinesToNodes}
                                };

                                out = ImGuiTreeNodeFlags_None; // Reset before applying new flags
                                for (size_t i = 0; i < arr->size(); ++i)
                                {
                                    if (auto optVal = (*arr)[i].value<std::string>())
                                    {
                                        std::string flagStr = *optVal;
                                        std::transform(flagStr.begin(), flagStr.end(), flagStr.begin(), ::tolower);
                                        
                                        auto it = flagMap.find(flagStr);
                                        if (it != flagMap.end())
                                        {
                                            out = static_cast<ImGuiTreeNodeFlags>(out | it->second); // Bitwise OR to accumulate flags
                                        }
                                    }
                                }
                            }
                        }
                    };
                    getTreeNodeFlags("TreeLinesFlags", style.TreeLinesFlags);

                    auto getHoveredFlags = [&](const char *key, ImGuiHoveredFlags &out)
                    {
                        if (auto node = findCaseInsensitive(styleTable, key))
                        {
                            if (auto arr = node->as_array())
                            {
                                static const std::unordered_map<std::string_view, ImGuiHoveredFlags> flagMap = {
                                    {"none", ImGuiHoveredFlags_None},
                                    {"childwindows", ImGuiHoveredFlags_ChildWindows},
                                    {"rootwindow", ImGuiHoveredFlags_RootWindow},
                                    {"anywindow", ImGuiHoveredFlags_AnyWindow},
                                    {"nopopuphierarchy", ImGuiHoveredFlags_NoPopupHierarchy},
                                    {"dockhierarchy", ImGuiHoveredFlags_DockHierarchy},
                                    {"allowwhenblockedbypopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup},
                                    {"allowwhenblockedbyactiveitem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem},
                                    {"allowwhenoverlappedbyitem", ImGuiHoveredFlags_AllowWhenOverlappedByItem},
                                    {"allowwhenoverlappedbywindow", ImGuiHoveredFlags_AllowWhenOverlappedByWindow},
                                    {"allowwhendisabled", ImGuiHoveredFlags_AllowWhenDisabled},
                                    {"nonavoverride", ImGuiHoveredFlags_NoNavOverride},
                                    {"allowwhenoverlapped", ImGuiHoveredFlags_AllowWhenOverlapped},
                                    {"rectonly", ImGuiHoveredFlags_RectOnly},
                                    {"rootandchildwindows", ImGuiHoveredFlags_RootAndChildWindows},
                                    {"fortooltip", ImGuiHoveredFlags_ForTooltip},
                                    {"stationary", ImGuiHoveredFlags_Stationary},
                                    {"delaynone", ImGuiHoveredFlags_DelayNone},
                                    {"delayshort", ImGuiHoveredFlags_DelayShort},
                                    {"delaynormal", ImGuiHoveredFlags_DelayNormal},
                                    {"noshareddelay", ImGuiHoveredFlags_NoSharedDelay}
                                };

                                out = ImGuiHoveredFlags_None; // Reset before applying new flags
                                for (size_t i = 0; i < arr->size(); ++i)
                                {
                                    if (auto optVal = (*arr)[i].value<std::string>())
                                    {
                                        std::string flagStr = *optVal;
                                        std::transform(flagStr.begin(), flagStr.end(), flagStr.begin(), ::tolower);
                                        
                                        auto it = flagMap.find(flagStr);
                                        if (it != flagMap.end())
                                        {
                                            out = static_cast<ImGuiHoveredFlags>(out | it->second); // Bitwise OR to accumulate flags
                                        }
                                    }
                                }
                            }
                        }
                    };
                    getHoveredFlags("HoverFlagsForTooltipMouse", style.HoverFlagsForTooltipMouse);
                    getHoveredFlags("HoverFlagsForTooltipNav", style.HoverFlagsForTooltipNav);
                }
            }

            // Apply DPI scaling before parsing colors
            style.ScaleAllSizes(dpiScale);

            // Parse colors
            if (auto colorsNodeRaw = findCaseInsensitive(themeTable, "colors"))
            {
                if (auto colorsTable = colorsNodeRaw->as_table())
                {
                    ImVec4 *colors = style.Colors;

                    static const std::unordered_map<std::string_view, ImGuiCol> colorMap = {
                        {"text", ImGuiCol_Text}, {"textdisabled", ImGuiCol_TextDisabled}, {"windowbg", ImGuiCol_WindowBg}, {"childbg", ImGuiCol_ChildBg}, {"popupbg", ImGuiCol_PopupBg}, {"border", ImGuiCol_Border}, {"bordershadow", ImGuiCol_BorderShadow}, {"framebg", ImGuiCol_FrameBg}, {"framebghovered", ImGuiCol_FrameBgHovered}, {"framebgactive", ImGuiCol_FrameBgActive}, {"titlebg", ImGuiCol_TitleBg}, {"titlebgactive", ImGuiCol_TitleBgActive}, {"titlebgcollapsed", ImGuiCol_TitleBgCollapsed}, {"menubarbg", ImGuiCol_MenuBarBg}, {"scrollbarbg", ImGuiCol_ScrollbarBg}, {"scrollbargrab", ImGuiCol_ScrollbarGrab}, {"scrollbargrabhovered", ImGuiCol_ScrollbarGrabHovered}, {"scrollbargrabactive", ImGuiCol_ScrollbarGrabActive}, {"checkmark", ImGuiCol_CheckMark}, {"slidergrab", ImGuiCol_SliderGrab}, {"slidergrabactive", ImGuiCol_SliderGrabActive}, {"button", ImGuiCol_Button}, {"buttonhovered", ImGuiCol_ButtonHovered}, {"buttonactive", ImGuiCol_ButtonActive}, {"header", ImGuiCol_Header}, {"headerhovered", ImGuiCol_HeaderHovered}, {"headeractive", ImGuiCol_HeaderActive}, {"separator", ImGuiCol_Separator}, {"separatorhovered", ImGuiCol_SeparatorHovered}, {"separatoractive", ImGuiCol_SeparatorActive}, {"resizegrip", ImGuiCol_ResizeGrip}, {"resizegriphovered", ImGuiCol_ResizeGripHovered}, {"resizegripactive", ImGuiCol_ResizeGripActive}, {"inputtextcursor", ImGuiCol_InputTextCursor}, {"tabhovered", ImGuiCol_TabHovered}, {"tab", ImGuiCol_Tab}, {"tabselected", ImGuiCol_TabSelected}, {"tabactive", ImGuiCol_TabSelected}, // Legacy alias
                        {"tabselectedoverline", ImGuiCol_TabSelectedOverline},
                        {"tabdimmed", ImGuiCol_TabDimmed},
                        {"tabunfocused", ImGuiCol_TabDimmed}, // Legacy alias
                        {"tabdimmedselected", ImGuiCol_TabDimmedSelected},
                        {"tabunfocusedactive", ImGuiCol_TabDimmedSelected}, // Legacy alias
                        {"tabdimmedselectedoverline", ImGuiCol_TabDimmedSelectedOverline},
                        {"dockingpreview", ImGuiCol_DockingPreview},
                        {"dockingemptybg", ImGuiCol_DockingEmptyBg},
                        {"plotlines", ImGuiCol_PlotLines},
                        {"plotlineshovered", ImGuiCol_PlotLinesHovered},
                        {"plothistogram", ImGuiCol_PlotHistogram},
                        {"plothistogramhovered", ImGuiCol_PlotHistogramHovered},
                        {"tableheaderbg", ImGuiCol_TableHeaderBg},
                        {"tableborderstrong", ImGuiCol_TableBorderStrong},
                        {"tableborderlight", ImGuiCol_TableBorderLight},
                        {"tablerowbg", ImGuiCol_TableRowBg},
                        {"tablerowbgalt", ImGuiCol_TableRowBgAlt},
                        {"textlink", ImGuiCol_TextLink},
                        {"textselectedbg", ImGuiCol_TextSelectedBg},
                        {"dragdroptarget", ImGuiCol_DragDropTarget},
                        {"dragdroptargetbg", ImGuiCol_DragDropTargetBg},
                        {"navcursor", ImGuiCol_NavCursor},
                        {"navhighlight", ImGuiCol_NavCursor}, // Legacy alias
                        {"navwindowinghighlight", ImGuiCol_NavWindowingHighlight},
                        {"navwindowingdimbg", ImGuiCol_NavWindowingDimBg},
                        {"modalwindowdimbg", ImGuiCol_ModalWindowDimBg}};

#ifdef DEBUG
                    // Assert in code that the map actually contains every ImGuiCol enumeration
                    static bool colorMapValidated = false;
                    if (!colorMapValidated)
                    {
                        bool mapped[ImGuiCol_COUNT] = {false};
                        for (const auto &kv : colorMap)
                            if (kv.second >= 0 && kv.second < ImGuiCol_COUNT)
                                mapped[kv.second] = true;
                        for (int i = 0; i < ImGuiCol_COUNT; ++i)
                            IM_ASSERT(mapped[i] && "A color is missing from the C++ colorMap! 100% enum coverage required.");
                        colorMapValidated = true;
                    }
#endif

                    for (const auto &[key, node] : *colorsTable)
                    {
                        std::string lowerKey = std::string(key.str());
                        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

                        auto it = colorMap.find(lowerKey);
                        if (it != colorMap.end())
                        {
                            if (auto arr = node.as_array())
                            {
                                if (arr->size() >= 3)
                                {
                                    float r = (*arr)[0].value_or<float>(0.0f);
                                    float g = (*arr)[1].value_or<float>(0.0f);
                                    float b = (*arr)[2].value_or<float>(0.0f);
                                    float a = arr->size() >= 4 ? (*arr)[3].value_or<float>(1.0f) : 1.0f;
                                    colors[it->second] = ImVec4(r, g, b, a);
                                }
                            }
                            else if (auto str = node.as_string())
                            {
                                std::string hex = str->get();
                                if (hex.length() >= 7 && hex[0] == '#')
                                {
                                    unsigned int r = 0, g = 0, b = 0, a = 255;
                                    if (hex.length() >= 9)
                                        sscanf(hex.c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);
                                    else
                                        sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
                                    colors[it->second] = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }
        catch (const toml::parse_error &)
        {
            return false;
        }
    }

    void Theme::applyComplementary()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            ImVec4& col = colors[i];
            float h, s, v;
            ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);
            
            h = fmod(h + 0.5f, 1.0f);
            
            ImGui::ColorConvertHSVtoRGB(h, s, v, col.x, col.y, col.z);
        }
    }

    void Theme::applyGrayscale()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            ImVec4& col = colors[i];
            // Perceptual luminance formula
            float gray = 0.299f * col.x + 0.587f * col.y + 0.114f * col.z;
            col.x = gray;
            col.y = gray;
            col.z = gray;
        }
    }

    void Theme::applySepia()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            ImVec4& col = colors[i];
            float r = col.x;
            float g = col.y;
            float b = col.z;

            col.x = ImMin(1.0f, (r * 0.393f) + (g * 0.769f) + (b * 0.189f));
            col.y = ImMin(1.0f, (r * 0.349f) + (g * 0.686f) + (b * 0.168f));
            col.z = ImMin(1.0f, (r * 0.272f) + (g * 0.534f) + (b * 0.131f));
        }
    }

    void Theme::applyInvert()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            ImVec4& col = colors[i];
            col.x = 1.0f - col.x;
            col.y = 1.0f - col.y;
            col.z = 1.0f - col.z;
        }
    }
} // namespace mui