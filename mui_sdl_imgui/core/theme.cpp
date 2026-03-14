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

// Include the macro definitions for the icons
// (Ensure this file is in your include path)
#include "IconsFontAwesome6.h"

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

        // Path to your TTF file (adjust this relative to your working directory)
        const std::string iconFontPath = "fonts/fa-solid-900.ttf";

        if (fileExists(iconFontPath))
        {
            io.Fonts->AddFontFromFileTTF(iconFontPath.c_str(), fontSize, &icon_config, icon_ranges);
        }
    }

    void Theme::applyStyle(float dpiScale)
    {
        ImGuiStyle &style = ImGui::GetStyle();

        // --- ENFORCE ANTI-ALIASING ---
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        style.AntiAliasedLinesUseTex = true; // Use texture-based AA for lines (smoother)

        // Geometry & Rounding (Base unscaled values)
        style.WindowRounding = 12.0f;
        style.ChildRounding = 12.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 8.0f;
        style.ScrollbarRounding = 12.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        // Erase Borders
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 0.0f;
        style.PopupBorderSize = 0.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;

        // Spacing & Padding
        style.ItemSpacing = ImVec2(8.0f, 8.0f);
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
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
    }
} // namespace mui