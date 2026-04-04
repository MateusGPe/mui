// widgets/breadcrumb.cpp
#include "breadcrumb.hpp"
#include "../core/app.hpp"
#include "../core/scoped.hpp"
#include "IconsFontAwesome6.h"
#include "entry.hpp"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>

#ifndef ICON_FA_ANGLE_RIGHT
#define ICON_FA_ANGLE_RIGHT "\xef\x84\x85"
#endif

namespace mui
{
    BreadcrumbBar::BreadcrumbBar(const std::string &path)
    {
        App::assertMainThread();
        inlineShadowEnabled = false;
        currentPath = path;
        parsePath();
        m_editEntry = Entry::create(editBuffer, sizeof(editBuffer))
                          ->setAutoSelectAll(true)
                          ->onEnter(
                              [this](const std::string &text)
                              {
                                  setPath(text);
                                  setIsEditing(false);
                                  onPathNavigatedSignal(currentPath);
                              });
    }

    void BreadcrumbBar::parsePath()
    {
        segments.clear();
        std::filesystem::path p(currentPath);
        for (auto it = p.begin(); it != p.end(); ++it)
        {
            std::string segment = it->string();
            if (segment != "\\" && segment != "/")
            {
                segments.push_back(segment);
            }
        }
    }

    void BreadcrumbBar::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        ImGuiStyle &style = ImGui::GetStyle();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        float natural_w = avail.x;
        float natural_h = ImGui::GetFrameHeight();
        ImVec2 actualSize = ApplySizeConstraints(ImVec2(natural_w, natural_h));

        // Get absolute position to calculate hover over the child rect
        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos, ImVec2(pos.x + actualSize.x, pos.y + actualSize.y));

        bool hovered = ImGui::IsMouseHoveringRect(bb.Min, bb.Max);
        bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);

        // Use Child Window to perfectly encapsulate the cursor changes and clipping
        ScopedColor childBg(
            ImGuiCol_ChildBg,
            style.Colors[hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]);
        ScopedStyle styleVars;
        styleVars.push(ImGuiStyleVar_ChildRounding, style.FrameRounding);
        styleVars.push(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::BeginChild("##bc_host", actualSize, false,
                          ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse);

        if (!isEditing)
        {
            // --- 1. Measurement Phase ---
            if (m_segmentsDirty ||
                std::abs(actualSize.x - m_lastAvail.x) > 0.5f)
            {
                m_cachedSegmentWidths.clear();
                for (const auto &segment : segments)
                {
                    if (segment.empty())
                    {
                        m_cachedSegmentWidths.push_back(0.0f);
                        continue;
                    }
                    float button_width = ImGui::CalcTextSize(segment.c_str()).x + style.FramePadding.x * 2.0f;
                    m_cachedSegmentWidths.push_back(button_width);
                }
                m_segmentsDirty = false;
                m_lastAvail = actualSize;
            }

            float total_width = 0.0f;
            float separator_width = ImGui::GetFrameHeight(); // Separator is an IconButton with this width

            for (float w : m_cachedSegmentWidths)
            {
                total_width += w;
            }
            total_width += (segments.size() + 1) * separator_width;

            // --- 2. Elision & Rendering ---
            ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing,
                                    ImVec2(0.0f, style.ItemSpacing.y));
            bool anyItemInteraction = false;

            // --- Style for flat buttons to enhance the design ---
            ScopedColor button_colors;
            ScopedStyle button_style;
            ImVec4 hover_color, active_color;
            if (App::getTheme() == ThemeType::Dark)
            {
                hover_color = {1.0f, 1.0f, 1.0f, 0.1f};
                active_color = {1.0f, 1.0f, 1.0f, 0.15f};
            }
            else
            {
                hover_color = {0.0f, 0.0f, 0.0f, 0.05f};
                active_color = {0.0f, 0.0f, 0.0f, 0.1f};
            }
            button_colors.push(ImGuiCol_Button, {0, 0, 0, 0});
            button_colors.push(ImGuiCol_ButtonHovered, hover_color);
            button_colors.push(ImGuiCol_ButtonActive, active_color);
            button_style.push(ImGuiStyleVar_FrameBorderSize, 0.0f);
            button_style.push(ImGuiStyleVar_FrameRounding, 4.0f);

            // Helper to reconstruct and navigate to a path from segments
            auto navigate_to_segment = [&](size_t segment_idx)
            {
                std::string newPath = "";
#ifndef _WIN32
                // Restore the absolute path root for non-Windows platforms
                if (!segments.empty() && segments[0] != "This PC" &&
                    segments[0] != "Quick Access")
                    newPath = "/";
#endif
                for (size_t j = 0; j <= segment_idx; ++j)
                {
                    newPath += segments[j];
#ifdef _WIN32
                    if (j < segment_idx)
                        newPath += "\\";
#else
                    if (j < segment_idx)
                        newPath += "/";
#endif
                }

#ifdef _WIN32
                if (newPath.size() == 2 && newPath[1] == ':')
                {
                    newPath += "\\";
                }
#endif

                setPath(newPath);
                onPathNavigatedSignal(currentPath);
            };

            // Helper to render a segment button
            auto render_segment_button = [&](size_t i)
            {
                ScopedID btn_id(static_cast<int>(i));
                if (ImGui::Button(segments[i].c_str(),
                                  ImVec2(m_cachedSegmentWidths[i], actualSize.y)))
                {
                    navigate_to_segment(i);
                }
                anyItemInteraction |= ImGui::IsItemHovered();
            };

            // Helper to render a separator
            auto render_separator = [&](int idx)
            {
                ScopedID sep_id("sep" + std::to_string(idx));

                if (ImGui::Button(ICON_FA_ANGLE_RIGHT,
                                  ImVec2(separator_width, actualSize.y)))
                {
                    ImGui::OpenPopup("BreadcrumbSubdirs");
                }
                anyItemInteraction |= ImGui::IsItemHovered();

                if (ImGui::BeginPopup("BreadcrumbSubdirs"))
                {
                    std::string levelPath = "";
                    if (segments.empty())
                    {
#ifdef _WIN32
                        levelPath = "This PC";
#else
                        levelPath = "/";
#endif
                    }
#ifndef _WIN32
                    if (!segments.empty() && segments[0] != "This PC" &&
                        segments[0] != "Quick Access")
                        levelPath = "/";
#endif
                    for (int j = 0; j < idx; ++j)
                    {
                        levelPath += segments[j];
#ifdef _WIN32
                        levelPath += "\\";
#else
                        levelPath += "/";
#endif
                    }

                    std::error_code ec;
                    bool found_any = false;

                    if (std::filesystem::is_directory(levelPath, ec))
                    {
                        std::vector<std::filesystem::path> subdirs;
                        try
                        {
                            for (const auto &entry : std::filesystem::directory_iterator(
                                     levelPath,
                                     std::filesystem::directory_options::skip_permission_denied,
                                     ec))
                            {
                                if (!ec && entry.is_directory(ec))
                                {
                                    subdirs.push_back(entry.path());
                                }
                            }
                        }
                        catch (...)
                        {
                            // Suppress outright catastrophic directory open failures
                        }

                        std::sort(subdirs.begin(), subdirs.end());

                        for (const auto &p : subdirs)
                        {
                            found_any = true;
                            if (ImGui::MenuItem(p.filename().string().c_str()))
                            {
                                setPath(p.string());
                                onPathNavigatedSignal(currentPath);
                            }
                        }
                    }

                    if (!found_any)
                    {
                        ImGui::TextDisabled("No subfolders");
                    }
                    ImGui::EndPopup();
                }
            };

            if (total_width <= actualSize.x || segments.size() < 3)
            {
                // --- Render all segments ---
                for (size_t i = 0; i < segments.size(); ++i)
                {
                    render_separator(i);
                    ImGui::SameLine();
                    render_segment_button(i);
                    ImGui::SameLine();
                }
                render_separator(segments.size());
            }
            else
            {
                // --- Render with middle elision ---
                float ellipsis_width =
                    ImGui::CalcTextSize("...").x + style.FramePadding.x * 2.0f;
                // Available space for the block of segments at the end, including their
                // preceding separators. The layout is [sep] [seg0] [sep] [...]
                // [last_block] [sep]. The last_block is rendered as [sep][seg]... so the
                // separator is part of the block.
                float available_for_last =
                    actualSize.x -
                    (separator_width + m_cachedSegmentWidths[0] + separator_width) -
                    ellipsis_width - separator_width;

                size_t last_segment_start_idx = segments.size();
                float last_segments_width = 0.0f;

                for (int i = segments.size() - 1; i > 0; --i)
                {
                    // The width needed for a segment includes the separator before it.
                    float needed = m_cachedSegmentWidths[i] + separator_width;
                    if (last_segments_width + needed > available_for_last)
                        break;
                    last_segments_width += needed;
                    last_segment_start_idx = i;
                }

                // 1. Render first segment
                render_separator(0);
                ImGui::SameLine();
                render_segment_button(0);
                ImGui::SameLine();

                // 2. Render ellipsis and handle popup
                size_t first_elided_idx = 1;
                if (last_segment_start_idx > first_elided_idx)
                {
                    render_separator(first_elided_idx);
                    ImGui::SameLine();

                    ScopedID ellipsis_id("ellipsis");
                    if (ImGui::Button("...", ImVec2(ellipsis_width, actualSize.y)))
                    {
                        ImGui::OpenPopup("##breadcrumb_overflow_popup");
                    }
                    anyItemInteraction |= ImGui::IsItemHovered();
                    ImGui::SameLine();

                    if (ImGui::BeginPopup("##breadcrumb_overflow_popup"))
                    {
                        for (size_t i = first_elided_idx; i < last_segment_start_idx; ++i)
                        {
                            if (ImGui::MenuItem(
                                    (segments[i] + "##" + std::to_string(i)).c_str()))
                                navigate_to_segment(i);
                        }
                        ImGui::EndPopup();
                    }
                }

                // 3. Render last segments
                for (size_t i = last_segment_start_idx; i < segments.size(); ++i)
                {
                    render_separator(i);
                    ImGui::SameLine();
                    render_segment_button(i);
                    ImGui::SameLine();
                }
                render_separator(segments.size());
            }

            if (!anyItemInteraction && clicked)
            {
                setIsEditing(true);
            }
        }
        else
        {
            ImGui::SetKeyboardFocusHere();

            // The mui::Entry doesn't have a direct height property for single-line,
            // so we adjust the FramePadding to make it fill the vertical space.
            float expectedPaddingY = (actualSize.y - ImGui::GetTextLineHeight()) / 2.0f;
            ScopedStyle style_vars;
            if (expectedPaddingY > style.FramePadding.y)
            {
                style_vars.push(ImGuiStyleVar_FramePadding,
                                ImVec2(style.FramePadding.x, expectedPaddingY));
            }

            m_editEntry->setSpanAvailWidth(true);
            m_editEntry->render();

            if (!ImGui::IsItemActive() &&
                (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
            {
                setIsEditing(false);
            }
        }

        ImGui::EndChild();

        renderTooltip();
        ImGui::EndDisabled();
    }

    std::string BreadcrumbBar::getPath() const { return currentPath; }

    BreadcrumbBarPtr BreadcrumbBar::setPath(const std::string &path)
    {
        if (currentPath == path)
            return self();
        m_segmentsDirty = true;
        currentPath = path;
        parsePath();
        return self();
    }

    BreadcrumbBarPtr BreadcrumbBar::bind(
        std::shared_ptr<Observable<std::string>> observable)
    {
        setPath(observable->get());
        addConnection(
            observable->onValueChanged.connect([this](const std::string &val)
                                               { mui::App::queueMain([this, val]()
                                                                     { this->setPath(val); }); }));
        addConnection(onPathNavigatedSignal.connect(
            [observable](const std::string &val)
            { observable->set(val); }));
        return self();
    }

    BreadcrumbBarPtr BreadcrumbBar::onPathNavigated(
        std::function<void(const std::string &)> cb)
    {
        if (cb)
            addConnection(onPathNavigatedSignal.connect(std::move(cb)));
        return self();
    }

    BreadcrumbBarPtr BreadcrumbBar::setIsEditing(bool editing)
    {
        if (isEditing == editing)
            return self();

        isEditing = editing;
        if (editing)
        {
            std::strncpy(editBuffer, currentPath.c_str(), sizeof(editBuffer) - 1);
            editBuffer[sizeof(editBuffer) - 1] = '\0'; // Ensure null-termination
        }
        else
        {
            m_segmentsDirty = true;
        }
        return self();
    }
} // namespace mui
