// widgets/image.cpp
#include "image.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    Image::Image(ImTextureID tex, float w, float h) 
        : textureId(tex), width(w), height(h) { App::assertMainThread(); }

    void Image::render()
    {
        if (!visible || !textureId) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);
        
        ImGui::Image(textureId, ImVec2(width, height), uv0, uv1, tintColor, borderColor);
        
        renderTooltip();
        
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    ImagePtr Image::setTexture(ImTextureID tex) { textureId = tex; return self(); }
    ImagePtr Image::setSize(float w, float h) { width = w; height = h; return self(); }
    ImagePtr Image::setUV(float u0, float v0, float u1, float v1) { 
        uv0 = ImVec2(u0, v0); 
        uv1 = ImVec2(u1, v1); 
        return self(); 
    }
    ImagePtr Image::setTint(ImVec4 tint) { tintColor = tint; return self(); }
    ImagePtr Image::setBorder(ImVec4 border) { borderColor = border; return self(); }
} // namespace mui