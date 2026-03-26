// widgets/image.hpp
#pragma once
#include "control.hpp"
#include <memory>
#include <imgui.h>

namespace mui
{
    class Image;
    using ImagePtr = std::shared_ptr<Image>;

    class Image : public Control<Image>
    {
    protected:
        ImTextureID textureId;
        ImVec2 uv0 = ImVec2(0, 0);
        ImVec2 uv1 = ImVec2(1, 1);
        ImVec4 tintColor = ImVec4(1, 1, 1, 1);
        ImVec4 borderColor = ImVec4(0, 0, 0, 0);
        Image(ImTextureID tex, float w, float h);

    public:
        static ImagePtr create(ImTextureID tex, float w, float h) { return std::shared_ptr<Image>(new Image(tex, w, h)); }

        void renderControl() override;

        ImagePtr setTexture(ImTextureID tex);
        ImagePtr setUV(float u0, float v0, float u1, float v1);
        ImagePtr setTint(ImVec4 tint);
        ImagePtr setBorder(ImVec4 border);
    };
} // namespace mui