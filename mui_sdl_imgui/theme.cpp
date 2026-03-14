#include "theme.hpp"
#include <imgui.h>
#include <fstream>
#include <string>

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
        fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
#elif defined(__APPLE__)
        fontPath = "/System/Library/Fonts/Supplemental/Helvetica.ttc";
#elif defined(__linux__)
        // Array of common high-quality Linux sans-serif fonts
        const char *linuxFonts[] = {
            "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf"};
        for (const char *path : linuxFonts)
        {
            if (fileExists(path))
            {
                fontPath = path;
                break;
            }
        }
#endif

        if (!fontPath.empty() && fileExists(fontPath))
        {
            // High quality text rendering settings
            ImFontConfig config;
            config.OversampleH = 3;
            config.OversampleV = 1;
            config.PixelSnapH = true;
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, &config);
        }
        else
        {
            // Fallback if OS font cannot be resolved
            io.Fonts->AddFontDefault();
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
        style.ItemSpacing = ImVec2(12.0f, 12.0f);
        style.FramePadding = ImVec2(16.0f, 8.0f);
        style.WindowPadding = ImVec2(16.0f, 16.0f);

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