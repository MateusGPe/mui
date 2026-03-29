// widgets/imagestackview.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imagestackview.hpp"
#include "../core/app.hpp"
#include "../core/scoped.hpp"
#include "../include/IconsFontAwesome6.h"
#include <imgui_internal.h>
#include <cmath>

#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#ifndef ICON_FA_EYE
#define ICON_FA_EYE "\xef\x81\xae"
#endif
#ifndef ICON_FA_EYE_SLASH
#define ICON_FA_EYE_SLASH "\xef\x81\xb0"
#endif
#ifndef ICON_FA_ARROW_UP
#define ICON_FA_ARROW_UP "\xef\x81\xa2"
#endif
#ifndef ICON_FA_ARROW_DOWN
#define ICON_FA_ARROW_DOWN "\xef\x81\xa3"
#endif
#ifndef ICON_FA_TRASH
#define ICON_FA_TRASH "\xef\x81\x93"
#endif
#ifndef ICON_FA_PLAY
#define ICON_FA_PLAY "\xef\x81\x8b"
#endif
#ifndef ICON_FA_PAUSE
#define ICON_FA_PAUSE "\xef\x81\x8c"
#endif

namespace mui
{

    ImageStackView::ImageStackView()
    {
        App::assertMainThread();
    }

    ImageStackView::~ImageStackView()
    {
        clearLayers();
    }

    void ImageStackView::freeTexture(ImTextureID tex)
    {
        if (!tex)
            return;
        if (mui::g_use_opengl)
        {
            GLuint gl_tex = (GLuint)(intptr_t)tex;
            glDeleteTextures(1, &gl_tex);
        }
        else
        {
            SDL_DestroyTexture((SDL_Texture *)(intptr_t)tex);
        }
    }

    ImageStackViewPtr ImageStackView::addLayer(const std::string &name, ImTextureID tex, float w, float h, bool takeOwnership)
    {
        m_layers.push_back({name, tex, w, h, true, takeOwnership, 1.0f});
        if (m_firstRender && m_layers.size() == 1)
        {
            fitToView();
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::loadFromMemory(const std::string &name, int width, int height, int channels, const unsigned char *data)
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
            return self(); // Unsupported format
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            new_texture = (ImTextureID)(intptr_t)texture_id;
        }
        else
        {
            SDL_Texture *texture = SDL_CreateTexture(mui::g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
            if (texture)
            {
                SDL_UpdateTexture(texture, nullptr, data, width * 4);
            }
            new_texture = (ImTextureID)(intptr_t)texture;
        }

        if (new_texture)
        {
            addLayer(name, new_texture, (float)width, (float)height, true);
        }

        return self();
    }

    ImageStackViewPtr ImageStackView::removeLayer(int index)
    {
        if (index >= 0 && index < (int)m_layers.size())
        {
            if (m_layers[index].ownsTexture)
            {
                freeTexture(m_layers[index].textureId);
            }
            m_layers.erase(m_layers.begin() + index);
            if (m_selectedLayer >= (int)m_layers.size())
            {
                m_selectedLayer = std::max(0, (int)m_layers.size() - 1);
            }
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::moveLayerUp(int index)
    {
        if (index >= 0 && index < (int)m_layers.size() - 1)
        {
            std::swap(m_layers[index], m_layers[index + 1]);
            if (m_selectedLayer == index)
                m_selectedLayer++;
            else if (m_selectedLayer == index + 1)
                m_selectedLayer--;
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::moveLayerDown(int index)
    {
        if (index > 0 && index < (int)m_layers.size())
        {
            std::swap(m_layers[index], m_layers[index - 1]);
            if (m_selectedLayer == index)
                m_selectedLayer--;
            else if (m_selectedLayer == index - 1)
                m_selectedLayer++;
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::clearLayers()
    {
        for (auto &layer : m_layers)
        {
            if (layer.ownsTexture)
            {
                freeTexture(layer.textureId);
            }
        }
        m_layers.clear();
        m_selectedLayer = 0;
        return self();
    }

    void ImageStackView::updateZoom(ImVec2 canvasP0, ImVec2 canvasSize)
    {
        ImGuiIO &io = ImGui::GetIO();
        if (!ImGui::IsItemHovered())
            return;

        float wheel = io.MouseWheel;
        if (wheel == 0.0f)
            return;

        // Consume the mouse wheel so parent containers don't scroll while we zoom
        io.MouseWheel = 0.0f;

        float zoomRate = 0.15f;
        float oldScale = m_scale;

        ImVec2 mousePos = io.MousePos;
        ImVec2 imageOrigin = canvasP0 + m_translate;
        ImVec2 relativeMousePos = mousePos - imageOrigin;

        m_scale += wheel * zoomRate * m_scale;
        m_scale = std::clamp(m_scale, 0.01f, 100.0f);

        float scaleRatio = m_scale / oldScale;
        m_translate = mousePos - canvasP0 - (relativeMousePos * scaleRatio);
    }

    ImageStackViewPtr ImageStackView::fitToView()
    {
        m_firstRender = true;
        return self();
    }

    ImageStackViewPtr ImageStackView::resetZoom()
    {
        m_scale = 1.0f;
        m_translate = ImVec2(0, 0);
        return self();
    }

    ImageStackViewPtr ImageStackView::setSidebarWidth(float width)
    {
        m_sidebarWidth = width;
        return self();
    }
    
    ImageStackViewPtr ImageStackView::setSingleLayerMode(bool single)
    {
        m_singleLayerMode = single;
        m_firstRender = true;
        return self();
    }

    ImageStackViewPtr ImageStackView::setThumbnailMode(bool thumb)
    {
        m_thumbnailMode = thumb;
        return self();
    }

    ImageStackViewPtr ImageStackView::setSlideshowMode(bool slideshow, float interval)
    {
        m_slideshowMode = slideshow;
        m_slideshowInterval = interval;
        m_slideshowTimer = 0.0f;
        return self();
    }

    void ImageStackView::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        // Enclose the control in a child window that perfectly fits available space
        ImGuiChildFlags host_flags = 0;
        ImGuiWindowFlags host_win_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::BeginChild("##imagestack_host", ImVec2(0, 0), host_flags, host_win_flags);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 available_size = ImGui::GetContentRegionAvail();

        // Fallback size if in an auto-resizing container
        if (available_size.x <= 0)
            available_size.x = 400.0f;
        if (available_size.y <= 0)
            available_size.y = 300.0f;

        float scaledSidebarWidth = m_sidebarWidth * App::getDpiScale();
        ImVec2 view_size = ImVec2(std::max(10.0f, available_size.x - scaledSidebarWidth - ImGui::GetStyle().ItemSpacing.x), available_size.y);
        ImVec2 canvas_p1 = canvas_p0 + view_size;

        // 1. Draw Canvas Background
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(40, 40, 40, 255));

        // Checkerboard pattern for transparency indication
        int checker_size = 16;
        for (int y = 0; y < view_size.y; y += checker_size)
        {
            for (int x = 0; x < view_size.x; x += checker_size)
            {
                if (((x / checker_size) + (y / checker_size)) % 2 == 0)
                {
                    ImVec2 rect_min = canvas_p0 + ImVec2((float)x, (float)y);
                    ImVec2 rect_max = canvas_p0 + ImVec2(std::min((float)x + checker_size, view_size.x), std::min((float)y + checker_size, view_size.y));
                    draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(55, 55, 55, 255));
                }
            }
        }

        // 2. Interaction Logic
        ImGui::InvisibleButton("##canvas", view_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        bool is_active = ImGui::IsItemActive();

        if (m_layers.empty())
        {
            const char *text = "No Layers Loaded";
            ImVec2 text_size = ImGui::CalcTextSize(text);
            draw_list->AddText(canvas_p0 + (view_size - text_size) * 0.5f, IM_COL32(150, 150, 150, 255), text);
        }
        else
        {
            // Auto-advance slideshow timer
            if (m_slideshowMode && !m_layers.empty())
            {
                m_slideshowTimer += ImGui::GetIO().DeltaTime;
                if (m_slideshowTimer >= m_slideshowInterval)
                {
                    m_slideshowTimer = 0.0f;
                    m_selectedLayer--; // Decrement traverses top visually, simulating chronological
                    if (m_selectedLayer < 0)
                        m_selectedLayer = (int)m_layers.size() - 1;
                    m_firstRender = true; // Auto-fit new sizes
                }
            }

            float maxWidth = 0.0f;
            float maxHeight = 0.0f;
            if (m_singleLayerMode && m_selectedLayer >= 0 && m_selectedLayer < (int)m_layers.size())
            {
                maxWidth = m_layers[m_selectedLayer].width;
                maxHeight = m_layers[m_selectedLayer].height;
            }
            else
            {
                for (const auto &layer : m_layers)
                {
                    maxWidth = std::max(maxWidth, layer.width);
                    maxHeight = std::max(maxHeight, layer.height);
                }
            }

            if (m_firstRender)
            {
                if (maxWidth > 0 && maxHeight > 0)
                {
                    float scaleX = view_size.x / maxWidth;
                    float scaleY = view_size.y / maxHeight;
                    m_scale = std::min(scaleX, scaleY) * 0.95f;
                    m_translate = (view_size - (ImVec2(maxWidth, maxHeight) * m_scale)) * 0.5f;
                }
                m_firstRender = false;
            }

            updateZoom(canvas_p0, view_size);

            if (is_active && (ImGui::IsMouseDragging(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle)))
            {
                m_translate += ImGui::GetIO().MouseDelta;
            }

            // Constrain translation and center if image is smaller than viewport
            float scaled_w = maxWidth * m_scale;
            float scaled_h = maxHeight * m_scale;

            if (scaled_w < view_size.x)
            {
                m_translate.x = (view_size.x - scaled_w) * 0.5f;
            }
            else
            {
                m_translate.x = std::clamp(m_translate.x, view_size.x - scaled_w, 0.0f);
            }

            if (scaled_h < view_size.y)
            {
                m_translate.y = (view_size.y - scaled_h) * 0.5f;
            }
            else
            {
                m_translate.y = std::clamp(m_translate.y, view_size.y - scaled_h, 0.0f);
            }

            // 3. Render Layers
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);
            ImVec2 origin = canvas_p0 + m_translate;

            for (int i = 0; i < (int)m_layers.size(); ++i)
            {
                const auto &layer = m_layers[i];
                if (m_singleLayerMode) {
                    if (i != m_selectedLayer) continue;
                } else {
                    if (!layer.visible || layer.opacity <= 0.0f) continue;
                }

                ImVec2 p1 = origin + ImVec2(layer.width * m_scale, layer.height * m_scale);

                ImU32 col = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, m_singleLayerMode ? 1.0f : layer.opacity));
                draw_list->AddImage(layer.textureId, origin, p1, ImVec2(0, 0), ImVec2(1, 1), col);
            }
            draw_list->PopClipRect();
        }

        // Border
        //draw_list->AddRect(canvas_p0, canvas_p1, ImGui::GetColorU32(ImGuiCol_Border));

        // 4. Sidebar Logic
        ImGui::SameLine();
        ImGui::BeginChild("##layer_sidebar", ImVec2(scaledSidebarWidth, available_size.y), 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        renderSidebar(ImVec2(scaledSidebarWidth, available_size.y));
        ImGui::EndChild();

        ImGui::EndChild();
        renderTooltip();
        ImGui::EndDisabled();
    }

    void ImageStackView::renderSidebar(ImVec2 size)
    {
        ImGui::BeginGroup();
        ImGui::Text(ICON_FA_LAYER_GROUP);
        ImGui::EndGroup();

        // View mode toggles
        ImGui::SameLine(ImGui::GetWindowWidth() - 70 * App::getDpiScale());
        if (ImGui::Button(m_thumbnailMode ? ICON_FA_LIST : ICON_FA_GRIP)) {
            m_thumbnailMode = !m_thumbnailMode;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(m_thumbnailMode ? "Switch to List View" : "Switch to Grid View");

        ImGui::SameLine();
        if (ImGui::Button(m_singleLayerMode ? ICON_FA_IMAGES : ICON_FA_IMAGE)) {
            m_singleLayerMode = !m_singleLayerMode;
            m_firstRender = true; // Recalculate zoom bounding boxes 
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(m_singleLayerMode ? "Switch to Stack View" : "Switch to Single View");

        ImGui::Separator();

        // Use a negative height to dynamically reserve space for the bottom controls, avoiding double scrollbars
        float bottom_reserved = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 3.0f;
        ImGui::BeginChild("##layer_list", ImVec2(0, -bottom_reserved));

        if (m_thumbnailMode)
        {
            float content_w = ImGui::GetContentRegionAvail().x;
            float thumb_size = 70.0f * App::getDpiScale();
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            int columns = std::max(1, (int)(content_w / (thumb_size + spacing)));
            
            for (int i = (int)m_layers.size() - 1; i >= 0; --i)
            {
                ImGui::PushID(i);
                bool isSelected = (m_selectedLayer == i);

                ImVec2 cursor = ImGui::GetCursorScreenPos();
                if (isSelected) {
                    ImGui::GetWindowDrawList()->AddRectFilled(cursor, cursor + ImVec2(thumb_size, thumb_size), ImGui::GetColorU32(ImGuiCol_Header));
                    ImGui::GetWindowDrawList()->AddRect(cursor, cursor + ImVec2(thumb_size, thumb_size), ImGui::GetColorU32(ImGuiCol_Border));
                }

                if (ImGui::InvisibleButton("##thumb", ImVec2(thumb_size, thumb_size))) {
                    m_selectedLayer = i;
                    m_slideshowTimer = 0.0f; // Reset slideshow on click
                }

                float aspect = m_layers[i].width / (m_layers[i].height == 0 ? 1.0f : m_layers[i].height);
                ImVec2 draw_size = ImVec2(thumb_size, thumb_size);
                if (aspect > 1.0f)
                    draw_size.y = thumb_size / aspect;
                else
                    draw_size.x = thumb_size * aspect;
                
                float pad = 4.0f * App::getDpiScale();
                draw_size -= ImVec2(pad*2, pad*2);
                
                ImVec2 thumb_offset = ImVec2((thumb_size - draw_size.x) * 0.5f, (thumb_size - draw_size.y) * 0.5f);
                
                // Ghost out thumbnail if visibility is technically toggled off, even in list view
                ImU32 col = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, m_layers[i].visible ? 1.0f : 0.3f));
                ImGui::GetWindowDrawList()->AddImage(
                    m_layers[i].textureId,
                    cursor + thumb_offset,
                    cursor + thumb_offset + draw_size,
                    ImVec2(0,0), ImVec2(1,1), col);

                if (ImGui::BeginPopupContextItem("##layer_ctx"))
                {
                    if (ImGui::MenuItem("Move Up", nullptr, false, i < (int)m_layers.size() - 1))
                        moveLayerUp(i);
                    if (ImGui::MenuItem("Move Down", nullptr, false, i > 0))
                        moveLayerDown(i);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete"))
                        removeLayer(i);
                    ImGui::EndPopup();
                }

                ImGui::PopID();
                
                int rev_idx = (int)m_layers.size() - 1 - i;
                if ((rev_idx + 1) % columns != 0 && i != 0) {
                    ImGui::SameLine();
                }
            }
        }
        else
        {
            // Traverse backwards so top layer is drawn at the top of the list
            for (int i = (int)m_layers.size() - 1; i >= 0; --i)
            {
                ImGui::PushID(i);
                bool isSelected = (m_selectedLayer == i);

                // Background for selected item
                if (isSelected)
                {
                    ImVec2 p_min = ImGui::GetCursorScreenPos();
                    ImVec2 p_max = ImVec2(p_min.x + ImGui::GetContentRegionAvail().x, p_min.y + 60 * App::getDpiScale());
                    ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, ImGui::GetColorU32(ImGuiCol_Header));
                }

                ImGui::BeginGroup();

                // Visibility toggle
                if (ImGui::Button(m_layers[i].visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH))
                {
                    m_layers[i].visible = !m_layers[i].visible;
                }

                ImGui::SameLine();

                // Aspect-correct Thumbnail calculation
                float thumb_size = 40.0f * App::getDpiScale();
                ImVec2 avail_thumb(thumb_size, thumb_size);
                float aspect = m_layers[i].width / (m_layers[i].height == 0 ? 1.0f : m_layers[i].height);
                ImVec2 draw_size = avail_thumb;
                if (aspect > 1.0f)
                    draw_size.y = thumb_size / aspect;
                else
                    draw_size.x = thumb_size * aspect;

                ImVec2 cursor = ImGui::GetCursorScreenPos();
                ImVec2 thumb_offset = ImVec2((thumb_size - draw_size.x) * 0.5f, (thumb_size - draw_size.y) * 0.5f);

                ImGui::GetWindowDrawList()->AddImage(
                    m_layers[i].textureId,
                    cursor + thumb_offset,
                    cursor + thumb_offset + draw_size);

                ImGui::Dummy(avail_thumb); // Reserve space for the manually drawn thumbnail

                ImGui::SameLine();

                // Layer properties (Name and Opacity)
                ImGui::BeginGroup();
                if (ImGui::Selectable(m_layers[i].name.c_str(), isSelected, ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 0)))
                {
                    m_selectedLayer = i;
                    m_slideshowTimer = 0.0f; // Reset slideshow on click
                }

                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 5);
                ImGui::SliderFloat("##op", &m_layers[i].opacity, 0.0f, 1.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::EndGroup();

                ImGui::EndGroup();

                // Quick context actions on right click
                if (ImGui::BeginPopupContextItem("##layer_ctx"))
                {
                    if (ImGui::MenuItem("Move Up", nullptr, false, i < (int)m_layers.size() - 1))
                        moveLayerUp(i);
                    if (ImGui::MenuItem("Move Down", nullptr, false, i > 0))
                        moveLayerDown(i);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete"))
                        removeLayer(i);
                    ImGui::EndPopup();
                }

                ImGui::PopID();
                ImGui::Separator();
            }
        }
        ImGui::EndChild();

        ImGui::Separator();

        // Bottom Controls
        ImGui::BeginDisabled(m_layers.empty() || m_selectedLayer < 0 || m_selectedLayer >= (int)m_layers.size());
        if (ImGui::Button(ICON_FA_ARROW_UP))
            moveLayerUp(m_selectedLayer);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN))
            moveLayerDown(m_selectedLayer);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TRASH))
            removeLayer(m_selectedLayer);
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button(m_slideshowMode ? ICON_FA_PAUSE : ICON_FA_PLAY)) {
            m_slideshowMode = !m_slideshowMode;
            if (m_slideshowMode) m_slideshowTimer = 0.0f;
            if (m_slideshowMode && !m_singleLayerMode) {
                m_singleLayerMode = true; // Auto-switch to single mode for slideshow
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Fit"))
            fitToView();
    }

} // namespace mui