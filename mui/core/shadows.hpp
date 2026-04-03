// ImGuiShadows.hpp
#pragma once
#include <imgui.h>
#include <vector>

namespace ImGuiShadows
{
    // Configuration for the shadow texture generation
    struct ShadowTextureConfig
    {
        int TexCornerSize = 16;
        int TexEdgeSize = 1;
        float TexFalloffPower = 4.8f;
        float TexDistanceFieldOffset = 3.8f;
        bool TexBlur = true;
        int Padding = 2;
        int ConvexPadding = 8;
    };

    // Container for the generated texture and UVs
    struct ShadowTextureData
    {
        int Width = 0;
        int Height = 0;
        std::vector<unsigned char> PixelsRGBA32; // Raw 32-bit RGBA data
        ImVec4 Uvs[10];                          // 0-8: Rect 9-slice UVs, 9: Convex UV
    };

    // 1. Call this once during initialization to generate the pixel data.
    // 2. Upload the PixelsRGBA32 to your GPU (OpenGL/DirectX/Vulkan) and get an ImTextureID.
    ShadowTextureData GenerateShadowTexture(const ShadowTextureConfig &cfg);

    // Drawing APIs
    void DrawShadowRect(
        ImDrawList *draw_list,
        ImTextureID shadow_tex,
        const ImVec4 *uvs, // Pass ShadowTextureData::Uvs
        const ImVec2 &obj_min,
        const ImVec2 &obj_max,
        ImU32 shadow_col,
        float shadow_thickness,
        const ImVec2 &shadow_offset,
        float obj_rounding = 0.0f,
        bool fill_background = true); // Set to false to cut out the shape background

    void DrawShadowConvexPoly(
        ImDrawList *draw_list,
        ImTextureID shadow_tex,
        const ImVec4 *uvs, // Pass ShadowTextureData::Uvs
        const ImVec2 *points,
        int points_count,
        ImU32 shadow_col,
        float shadow_thickness,
        const ImVec2 &shadow_offset,
        bool fill_background = true);

    void DrawShadowCircle(
        ImDrawList *draw_list,
        ImTextureID shadow_tex,
        const ImVec4 *uvs,
        const ImVec2 &obj_center,
        float obj_radius,
        ImU32 shadow_col,
        float shadow_thickness,
        const ImVec2 &shadow_offset,
        int num_segments = 12,
        bool fill_background = true);
}
