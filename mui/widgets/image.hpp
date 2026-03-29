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
        std::string getTypeName() const override { return "Image"; }
        std::string name;
        ImTextureID textureId;
        ImVec2 uv0 = ImVec2(0, 0);
        ImVec2 uv1 = ImVec2(1, 1);
        ImVec4 tintColor = ImVec4(1, 1, 1, 1);
        ImVec4 borderColor = ImVec4(0, 0, 0, 0);
        bool ownsTexture = false;
        float opacity = 1.0f;

        Image(ImTextureID tex, float w, float h);

    public:
        ~Image() override;

        static ImagePtr create(const std::string &name = "", ImTextureID tex = 0, float w = 0, float h = 0)
        {
            auto img = std::shared_ptr<Image>(new Image(tex, w, h));
            img->setName(name);
            return img;
        }
        static ImagePtr loadFromMemory(const std::string &name, int width, int height, int channels, const unsigned char *data);

        void renderControl() override;

        ImagePtr setTexture(ImTextureID tex, bool takeOwnership = false);
        ImagePtr setUV(float u0, float v0, float u1, float v1);
        ImagePtr setTint(ImVec4 tint);
        ImagePtr setBorder(ImVec4 border);
        ImagePtr setOpacity(float o);
        float getOpacity() const;
        ImagePtr setName(const std::string &n);
        std::string getName() const;
        ImTextureID getTextureId() const;
    };
} // namespace mui