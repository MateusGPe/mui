// widgets/image.cpp
#include "image.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    Image::Image(ImTextureID tex, float w, float h)
        : textureId(tex)
    {
        App::assertMainThread();
        width = w;
        height = h;
    }

    void Image::renderControl()
    {
        if (!visible || !textureId)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGui::Image(textureId, ImVec2(width, height), uv0, uv1, tintColor, borderColor);

        renderTooltip();

        ImGui::EndDisabled();
    }

    ImagePtr Image::setTexture(ImTextureID tex)
    {
        textureId = tex;
        return self();
    }
    ImagePtr Image::setUV(float u0, float v0, float u1, float v1)
    {
        uv0 = ImVec2(u0, v0);
        uv1 = ImVec2(u1, v1);
        return self();
    }
    ImagePtr Image::setTint(ImVec4 tint)
    {
        tintColor = tint;
        return self();
    }
    ImagePtr Image::setBorder(ImVec4 border)
    {
        borderColor = border;
        return self();
    }
} // namespace mui