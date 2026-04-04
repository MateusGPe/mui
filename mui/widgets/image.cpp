// widgets/image.cpp
#include "image.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <imgui.h>
#include <vector>

#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

namespace mui
{
    Image::Image(ImTextureID tex, float w, float h) : textureId(tex)
    {
        App::assertMainThread();
        width = w;
        height = h;
    }

    Image::~Image()
    {
        if (ownsTexture && textureId)
        {
            if (mui::g_use_opengl)
            {
                GLuint tex = (GLuint)(intptr_t)textureId;
                glDeleteTextures(1, &tex);
            }
            else
            {
                SDL_DestroyTexture((SDL_Texture *)(intptr_t)textureId);
            }
        }
    }

    void Image::renderControl()
    {
        if (!visible || !textureId)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImVec4 finalTintColor = tintColor;
        finalTintColor.w *= opacity;

        ImVec2 final_size = ApplySizeConstraints(ImVec2(width, height));
        ImGui::Image(textureId, final_size, uv0, uv1, finalTintColor,
                     borderColor);

        renderTooltip();

        ImGui::EndDisabled();
    }

    ImagePtr Image::setTexture(ImTextureID tex, bool takeOwnership)
    {
        if (ownsTexture && textureId && textureId != tex)
        {
            if (mui::g_use_opengl)
            {
                GLuint old_tex = (GLuint)(intptr_t)textureId;
                glDeleteTextures(1, &old_tex);
            }
            else
            {
                SDL_DestroyTexture((SDL_Texture *)(intptr_t)textureId);
            }
        }
        textureId = tex;
        ownsTexture = takeOwnership;
        return self();
    }

    ImagePtr Image::loadFromMemory(const std::string &name, int width, int height,
                                   int channels, const unsigned char *data)
    {
        std::vector<unsigned char> rgba_data;
        if (channels == 3)
        {
            rgba_data.resize(width * height * 4);
            for (int i = 0; i < width * height; ++i)
            {
                rgba_data[i * 4 + 0] = data[i * 3 + 0]; // R
                rgba_data[i * 4 + 1] = data[i * 3 + 1]; // G
                rgba_data[i * 4 + 2] = data[i * 3 + 2]; // B
                rgba_data[i * 4 + 3] = 255;             // A
            }
            data = rgba_data.data();
        }
        else if (channels != 4)
        {
            return nullptr; // Unsupported format
        }

        ImTextureID new_texture = 0;
        if (mui::g_use_opengl)
        {
            GLuint texture_id;
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, data);
            new_texture = (ImTextureID)(intptr_t)texture_id;
        }
        else
        {
            SDL_Texture *texture =
                SDL_CreateTexture(mui::g_renderer, SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_STATIC, width, height);
            if (texture)
            {
                SDL_UpdateTexture(texture, nullptr, data, width * 4);
            }
            new_texture = (ImTextureID)(intptr_t)texture;
        }

        if (new_texture)
        {
            auto img = Image::create(name, new_texture, (float)width, (float)height);
            img->ownsTexture = true;
            return img;
        }

        return nullptr;
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
    ImagePtr Image::setOpacity(float o)
    {
        opacity = o;
        return self();
    }
    float Image::getOpacity() const { return opacity; }
    ImagePtr Image::setName(const std::string &n)
    {
        name = n;
        setID(n);
        return self();
    }
    std::string Image::getName() const { return name; }

    ImTextureID Image::getTextureId() const { return textureId; }
} // namespace mui