#pragma once

#include <string>
#include <vector>

namespace mui
{
    class Theme
    {
    public:
        static void applyStyle(float dpiScale = 1.0f);
        static void applyDarkStyle(float dpiScale = 1.0f);
        static void loadSystemFont(float fontSize = 16.0f);
        static bool loadThemeFromToml(const std::string& filepath, const std::string& themeName, float dpiScale = 1.0f);
        static std::vector<std::string> getAvailableThemes(const std::string& filepath);
        static void applyGrayscale();
        static void applyComplementary();
        static void applySepia();
        static void applyInvert();
    };
} // namespace mui