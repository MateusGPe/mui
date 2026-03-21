#include "theme.hpp"
#include <imgui.h>
#include <fstream>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h> // For SHGetFolderPathA
#elif defined(__linux__)
#include <fontconfig/fontconfig.h>
#endif

#include "IconsFontAwesome6.h"
#include "fa_solid_900.hpp"

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
        style.ScrollbarRounding = 12.0f;
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
        style.FramePadding = ImVec2(8.0f, 8.0f);
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
        style.ScrollbarRounding = 12.0f;
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
} // namespace mui