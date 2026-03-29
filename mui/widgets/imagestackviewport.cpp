// widgets/imagestackviewport.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imagestackviewport.hpp"
#include "imagestackview.hpp" // Include the parent for state access
#include "../core/app.hpp"
#include <cmath>

namespace mui
{
    ImageStackViewport::ImageStackViewport(ImageStackView *parent) : m_parent(parent)
    {
        App::assertMainThread();
    }

    void ImageStackViewport::renderControl()
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 view_size = ImGui::GetContentRegionAvail();

        // Fallback size if in an auto-resizing container
        if (view_size.x <= 0)
            view_size.x = 400.0f;
        if (view_size.y <= 0)
            view_size.y = 300.0f;

        ImVec2 canvas_p1 = canvas_p0 + view_size;

        // 1. Draw Canvas Background
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));

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
                    draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(60, 60, 60, 255));
                }
            }
        }
        draw_list->AddRect(canvas_p0, canvas_p1, ImGui::GetColorU32(ImGuiCol_Border));

        // 2. Interaction Logic
        ImGui::InvisibleButton("##canvas", view_size, ImGuiButtonFlags_MouseButtonLeft);
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();

        if (m_parent->m_layers.empty())
        {
            const char *text = "No Layers Loaded";
            ImVec2 text_size = ImGui::CalcTextSize(text);
            draw_list->AddText(canvas_p0 + (view_size - text_size) * 0.5f, IM_COL32(150, 150, 150, 255), text);
        }
        else
        {
            float maxWidth = 0.0f;
            float maxHeight = 0.0f;
            if (m_parent->m_singleLayerMode && m_parent->m_selectedLayer >= 0 && m_parent->m_selectedLayer < (int)m_parent->m_layers.size())
            {
                maxWidth = m_parent->m_layers[m_parent->m_selectedLayer]->getWidth();
                maxHeight = m_parent->m_layers[m_parent->m_selectedLayer]->getHeight();
            }
            else
            {
                for (const auto &layer : m_parent->m_layers)
                {
                    maxWidth = std::max(maxWidth, layer->getWidth());
                    maxHeight = std::max(maxHeight, layer->getHeight());
                }
            }

            if (m_parent->m_firstRender)
            {
                if (maxWidth > 0 && maxHeight > 0)
                {
                    float scaleX = view_size.x / maxWidth;
                    float scaleY = view_size.y / maxHeight;
                    m_parent->m_scale = std::min(scaleX, scaleY) * 0.95f;
                    m_parent->m_translate = (view_size - (ImVec2(maxWidth, maxHeight) * m_parent->m_scale)) * 0.5f;
                }
                m_parent->m_firstRender = false;
            }

            // Zoom (center-based)
            if (is_hovered && ImGui::GetIO().MouseWheel != 0.0f)
            {
                float wheel = ImGui::GetIO().MouseWheel;
                float zoomRate = ImGui::GetIO().KeyShift ? 0.05f : 0.15f; // Slower zoom with Shift
                float oldScale = m_parent->m_scale;

                // The point we want to zoom into/out of is the center of the viewport
                ImVec2 canvas_center = canvas_p0 + view_size * 0.5f;

                // This is the position of the canvas center relative to the image's top-left corner (in screen space)
                ImVec2 image_space_center = canvas_center - (canvas_p0 + m_parent->m_translate);

                // Calculate the new scale
                float newScale = oldScale + wheel * zoomRate * oldScale;
                newScale = std::clamp(newScale, 0.01f, 100.0f);

                // The ratio of the new scale to the old scale
                float scaleRatio = newScale / oldScale;

                // To keep the center point stationary on screen, we adjust the translation
                m_parent->m_translate -= (image_space_center * scaleRatio) - image_space_center;
                m_parent->m_scale = newScale;
            }

            // Pan (left-mouse only)
            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                m_parent->m_translate += ImGui::GetIO().MouseDelta;
            }

            // Constrain translation
            float scaled_w = maxWidth * m_parent->m_scale;
            float scaled_h = maxHeight * m_parent->m_scale;
            if (scaled_w < view_size.x)
                m_parent->m_translate.x = (view_size.x - scaled_w) * 0.5f;
            else
                m_parent->m_translate.x = std::clamp(m_parent->m_translate.x, view_size.x - scaled_w, 0.0f);
            if (scaled_h < view_size.y)
                m_parent->m_translate.y = (view_size.y - scaled_h) * 0.5f;
            else
                m_parent->m_translate.y = std::clamp(m_parent->m_translate.y, view_size.y - scaled_h, 0.0f);

            // 3. Render Layers
            draw_list->PushClipRect(canvas_p0 + ImVec2(1, 1), canvas_p1 - ImVec2(1, 1), true);
            ImVec2 origin = canvas_p0 + m_parent->m_translate;

            for (int i = 0; i < (int)m_parent->m_layers.size(); ++i)
            {
                const auto &layer = m_parent->m_layers[i];
                if (m_parent->m_singleLayerMode)
                {
                    if (i != m_parent->m_selectedLayer)
                        continue;
                }
                else if (!layer->isVisible() || layer->getOpacity() <= 0.0f)
                {
                    continue;
                }

                ImVec2 p1 = origin + ImVec2(layer->getWidth() * m_parent->m_scale, layer->getHeight() * m_parent->m_scale);
                ImU32 col = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, m_parent->m_singleLayerMode ? 1.0f : layer->getOpacity()));
                draw_list->AddImage(layer->getTextureId(), origin, p1, ImVec2(0, 0), ImVec2(1, 1), col);
            }
            draw_list->PopClipRect();
        }
    }
} // namespace mui