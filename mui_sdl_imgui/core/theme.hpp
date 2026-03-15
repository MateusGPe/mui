#pragma once

namespace mui
{
    class Theme
    {
    public:
        static void applyStyle(float dpiScale = 1.0f);
        static void applyDarkStyle(float dpiScale = 1.0f);
        static void loadSystemFont(float fontSize = 16.0f);
    };
} // namespace mui