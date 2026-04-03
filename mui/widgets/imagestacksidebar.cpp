// widgets/imagestacksidebar.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imagestacksidebar.hpp"
#include "../core/app.hpp"
#include "../core/scoped.hpp"
#include "../include/IconsFontAwesome6.h"
#include "imagestackview.hpp"

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
#ifndef ICON_FA_LAYER_GROUP
#define ICON_FA_LAYER_GROUP "\xef\x97\xbd"
#endif
#ifndef ICON_FA_LIST
#define ICON_FA_LIST "\xef\x80\xba"
#endif
#ifndef ICON_FA_GRIP
#define ICON_FA_GRIP "\xef\x96\x8d"
#endif
#ifndef ICON_FA_IMAGES
#define ICON_FA_IMAGES "\xef\x8c\x82"
#endif
#ifndef ICON_FA_IMAGE
#define ICON_FA_IMAGE "\xef\x80\xbe"
#endif

namespace mui
{
    ImageStackSidebar::ImageStackSidebar(ImageStackView *parent)
        : m_parent(parent)
    {
        App::assertMainThread();
    }

    void ImageStackSidebar::renderControl()
    {
        ImGui::BeginGroup();
        ImGui::Text(ICON_FA_LAYER_GROUP);
        ImGui::EndGroup();

        // View mode toggles
        ImGui::SameLine(ImGui::GetWindowWidth() - 70 * App::getDpiScale());
        if (ImGui::Button(m_parent->m_thumbnailMode ? ICON_FA_LIST : ICON_FA_GRIP))
        {
            onToggleThumbnailMode(!m_parent->m_thumbnailMode);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip(m_parent->m_thumbnailMode ? "Switch to List View"
                                                        : "Switch to Grid View");

        ImGui::SameLine();
        if (ImGui::Button(m_parent->m_singleLayerMode ? ICON_FA_IMAGES
                                                      : ICON_FA_IMAGE))
        {
            onToggleSingleLayerMode(!m_parent->m_singleLayerMode);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip(m_parent->m_singleLayerMode ? "Switch to Stack View"
                                                          : "Switch to Single View");

        ImGui::Separator();

        // Use a negative height to dynamically reserve space for the bottom controls,
        // avoiding double scrollbars
        float bottom_reserved =
            ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 3.0f;
        ImGui::BeginChild("##layer_list", ImVec2(0, -bottom_reserved));

        if (m_parent->m_thumbnailMode)
        {
            float content_w = ImGui::GetContentRegionAvail().x;
            float thumb_size = 70.0f * App::getDpiScale();
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            int columns = std::max(1, (int)(content_w / (thumb_size + spacing)));

            for (int i = (int)m_parent->m_layers.size() - 1; i >= 0; --i)
            {
                ImGui::PushID(i);
                bool isSelected = (m_parent->m_selectedLayer == i);

                ImVec2 cursor = ImGui::GetCursorScreenPos();
                if (isSelected)
                {
                    ImGui::GetWindowDrawList()->AddRectFilled(
                        cursor, cursor + ImVec2(thumb_size, thumb_size),
                        ImGui::GetColorU32(ImGuiCol_Header));
                    ImGui::GetWindowDrawList()->AddRect(
                        cursor, cursor + ImVec2(thumb_size, thumb_size),
                        ImGui::GetColorU32(ImGuiCol_Border));
                }

                if (ImGui::InvisibleButton("##thumb", ImVec2(thumb_size, thumb_size)))
                {
                    onLayerSelect(i);
                }

                float aspect = m_parent->m_layers[i]->getWidth() /
                               (m_parent->m_layers[i]->getHeight() == 0
                                    ? 1.0f
                                    : m_parent->m_layers[i]->getHeight());
                ImVec2 draw_size = ImVec2(thumb_size, thumb_size);
                if (aspect > 1.0f)
                    draw_size.y = thumb_size / aspect;
                else
                    draw_size.x = thumb_size * aspect;

                float pad = 4.0f * App::getDpiScale();
                draw_size -= ImVec2(pad * 2, pad * 2);

                ImVec2 thumb_offset = ImVec2((thumb_size - draw_size.x) * 0.5f,
                                             (thumb_size - draw_size.y) * 0.5f);

                ImU32 col = ImGui::GetColorU32(ImVec4(
                    1.0f, 1.0f, 1.0f, m_parent->m_layers[i]->isVisible() ? 1.0f : 0.3f));
                ImGui::GetWindowDrawList()->AddImage(
                    m_parent->m_layers[i]->getTextureId(), cursor + thumb_offset,
                    cursor + thumb_offset + draw_size, ImVec2(0, 0), ImVec2(1, 1), col);

                if (ImGui::BeginPopupContextItem("##layer_ctx"))
                {
                    if (ImGui::MenuItem("Move Up", nullptr, false,
                                        i < (int)m_parent->m_layers.size() - 1))
                        onLayerMoveUp(i);
                    if (ImGui::MenuItem("Move Down", nullptr, false, i > 0))
                        onLayerMoveDown(i);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete"))
                        onLayerRemove(i);
                    ImGui::EndPopup();
                }

                ImGui::PopID();

                int rev_idx = (int)m_parent->m_layers.size() - 1 - i;
                if ((rev_idx + 1) % columns != 0 && i != 0)
                {
                    ImGui::SameLine();
                }
            }
        }
        else
        {
            for (int i = (int)m_parent->m_layers.size() - 1; i >= 0; --i)
            {
                ImGui::PushID(i);
                bool isSelected = (m_parent->m_selectedLayer == i);

                if (isSelected)
                {
                    ImVec2 p_min = ImGui::GetCursorScreenPos();
                    ImVec2 p_max = ImVec2(p_min.x + ImGui::GetContentRegionAvail().x,
                                          p_min.y + 60 * App::getDpiScale());
                    ImGui::GetWindowDrawList()->AddRectFilled(
                        p_min, p_max, ImGui::GetColorU32(ImGuiCol_Header));
                }

                ImGui::BeginGroup();

                if (ImGui::Button(m_parent->m_layers[i]->isVisible()
                                      ? ICON_FA_EYE
                                      : ICON_FA_EYE_SLASH))
                {
                    m_parent->m_layers[i]->setVisible(!m_parent->m_layers[i]->isVisible());
                }

                ImGui::SameLine();

                float thumb_size = 40.0f * App::getDpiScale();
                ImVec2 avail_thumb(thumb_size, thumb_size);
                float aspect = m_parent->m_layers[i]->getWidth() /
                               (m_parent->m_layers[i]->getHeight() == 0
                                    ? 1.0f
                                    : m_parent->m_layers[i]->getHeight());
                ImVec2 draw_size = avail_thumb;
                if (aspect > 1.0f)
                    draw_size.y = thumb_size / aspect;
                else
                    draw_size.x = thumb_size * aspect;

                ImVec2 cursor = ImGui::GetCursorScreenPos();
                ImVec2 thumb_offset = ImVec2((thumb_size - draw_size.x) * 0.5f,
                                             (thumb_size - draw_size.y) * 0.5f);

                ImGui::GetWindowDrawList()->AddImage(
                    m_parent->m_layers[i]->getTextureId(), cursor + thumb_offset,
                    cursor + thumb_offset + draw_size);
                ImGui::Dummy(avail_thumb);
                ImGui::SameLine();

                ImGui::BeginGroup();
                if (ImGui::Selectable(m_parent->m_layers[i]->getName().c_str(),
                                      isSelected, ImGuiSelectableFlags_AllowOverlap,
                                      ImVec2(0, 0)))
                {
                    onLayerSelect(i);
                }

                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 5);
                float opacity = m_parent->m_layers[i]->getOpacity();
                if (ImGui::SliderFloat("##op", &opacity, 0.0f, 1.0f, "%.2f"))
                {
                    m_parent->m_layers[i]->setOpacity(opacity);
                }
                ImGui::PopItemWidth();
                ImGui::EndGroup();

                ImGui::EndGroup();

                if (ImGui::BeginPopupContextItem("##layer_ctx"))
                {
                    if (ImGui::MenuItem("Move Up", nullptr, false,
                                        i < (int)m_parent->m_layers.size() - 1))
                        onLayerMoveUp(i);
                    if (ImGui::MenuItem("Move Down", nullptr, false, i > 0))
                        onLayerMoveDown(i);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete"))
                        onLayerRemove(i);
                    ImGui::EndPopup();
                }

                ImGui::PopID();
                ImGui::Separator();
            }
        }
        ImGui::EndChild();

        ImGui::Separator();

        ImGui::BeginDisabled(
            m_parent->m_layers.empty() || m_parent->m_selectedLayer < 0 ||
            m_parent->m_selectedLayer >= (int)m_parent->m_layers.size());
        if (ImGui::Button(ICON_FA_ARROW_UP))
            onLayerMoveUp(m_parent->m_selectedLayer);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN))
            onLayerMoveDown(m_parent->m_selectedLayer);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TRASH))
            onLayerRemove(m_parent->m_selectedLayer);
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button(m_parent->m_slideshowMode ? ICON_FA_PAUSE : ICON_FA_PLAY))
        {
            onToggleSlideshowMode(!m_parent->m_slideshowMode);
        }

        ImGui::SameLine();
        if (ImGui::Button("Fit"))
            onFitView();
    }
} // namespace mui