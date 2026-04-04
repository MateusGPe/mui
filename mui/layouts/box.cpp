// layouts/box.cpp
#include "box.hpp"
#include "../core/scoped.hpp"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>

namespace mui
{
    VBox::VBox() : Box() {}

    void VBox::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);

        ImGuiStyle &style = ImGui::GetStyle();
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 padding = padded ? style.WindowPadding : ImVec2(0, 0);

        if (scrollable)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
            if (padded)
                flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
            ImGui::BeginChild("##vbox_scroll", ImVec2(0, avail.y), 0, flags);
            ImGui::PopStyleVar();
            avail = ImGui::GetContentRegionAvail();
        }
        else
        {
            ImGui::BeginGroup();
            if (padded)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::Dummy(ImVec2(0, padding.y));
                ImGui::PopStyleVar();
                ImGui::Indent(padding.x);
            }
            avail.x -= padding.x * 2.0f;
            avail.y -= padding.y * 2.0f;
            avail.x = std::max(0.0f, avail.x);
            avail.y = std::max(0.0f, avail.y);
        }

        ImVec2 original_spacing = style.ItemSpacing;
        float item_spacing_y = (spacing >= 0.0f) ? spacing : original_spacing.y;

        std::vector<size_t> current_visible_indices;
        current_visible_indices.reserve(children.size());
        for (size_t i = 0; i < children.size(); ++i)
        {
            if (children[i].control->isVisible())
                current_visible_indices.push_back(i);
        }

        bool auto_size_changed = false;
        for (size_t i : m_cachedVisibleIndices)
        {
            auto &child = children[i];
            if (child.sizing.mode == Sizing::Mode::Auto)
            {
                if (std::abs(child.lastKnownSize.y - child.sizeUsedInLastLayout.y) > 0.5f)
                {
                    auto_size_changed = true;
                    break;
                }
            }
        }
        if (m_layoutDirty || std::abs(avail.y - m_lastAvail.y) > 0.5f || current_visible_indices != m_cachedVisibleIndices || auto_size_changed)
        {
            m_cachedVisibleIndices = current_visible_indices;
            m_cachedTotalStretchWeight = 0.0f;
            m_cachedFixedSize = 0.0f;

            float total_spacing = 0.0f;
            if (m_cachedVisibleIndices.size() > 1)
                total_spacing = (m_cachedVisibleIndices.size() - 1) * item_spacing_y;

            for (size_t i : m_cachedVisibleIndices)
            {
                auto &child = children[i];
                switch (child.sizing.mode)
                {
                case Sizing::Mode::Stretch:
                    m_cachedTotalStretchWeight += child.sizing.value;
                    break;
                case Sizing::Mode::Fixed:
                    m_cachedFixedSize += child.sizing.value;
                    break;
                case Sizing::Mode::Percent:
                    m_cachedFixedSize += avail.y * child.sizing.value;
                    break;
                case Sizing::Mode::Auto:
                    m_cachedFixedSize += child.lastKnownSize.y;
                    child.sizeUsedInLastLayout.y = child.lastKnownSize.y;
                    break;
                }
            }

            m_cachedStretchUnit = 0.0f;
            if (m_cachedTotalStretchWeight > 0)
            {
                float remaining_height = avail.y - m_cachedFixedSize - total_spacing;
                if (!scrollable)
                    remaining_height = std::max(0.0f, remaining_height);
                m_cachedStretchUnit = remaining_height / m_cachedTotalStretchWeight;
            }

            m_layoutDirty = false;
            m_lastAvail = avail;
        }

        const auto &visible_indices = m_cachedVisibleIndices;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(original_spacing.x, item_spacing_y));

        for (size_t idx = 0; idx < visible_indices.size(); ++idx)
        {
            size_t i = visible_indices[idx];
            auto &child = children[i];

            float child_height = 0.0f;
            bool use_child_window = false;

            switch (child.sizing.mode)
            {
            case Sizing::Mode::Stretch:
                child_height = m_cachedStretchUnit * child.sizing.value;
                if (!scrollable)
                    child_height = std::max(0.0f, child_height);
                use_child_window = true;
                break;
            case Sizing::Mode::Fixed:
                child_height = child.sizing.value;
                use_child_window = true;
                break;
            case Sizing::Mode::Percent:
                child_height = avail.y * child.sizing.value;
                use_child_window = true;
                break;
            case Sizing::Mode::Auto:
                break;
            }

            // Instead of buggy RAII variables, meticulously apply Native ImGui
            // functions preventing stack mismatches within loops
            ImGui::PushID(static_cast<int>(i));

            if (use_child_window)
            {
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings;
                if (!showChildScrollbars)
                    window_flags |= ImGuiWindowFlags_NoScrollbar;
                ImGui::BeginChild("##box_item", ImVec2(0, child_height), 0, window_flags);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, original_spacing);
                ImGui::PushItemWidth(-1.0f); // Stretch children fill width

                child.control->render();

                ImGui::PopItemWidth();
                ImGui::PopStyleVar();
                ImGui::EndChild();
            }
            else
            {
                ImGui::BeginGroup();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, original_spacing);
                if (fillWidth)
                    ImGui::PushItemWidth(-1.0f);

                child.control->render();

                if (fillWidth)
                    ImGui::PopItemWidth();
                ImGui::PopStyleVar();
                ImGui::EndGroup();
            }

            child.lastKnownSize = ImGui::GetItemRectSize();
            ImGui::PopID();
        }

        ImGui::PopStyleVar();

        if (scrollable && autoScroll &&
            ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }

        if (scrollable)
        {
            ImGui::EndChild();
        }
        else
        {
            if (padded)
            {
                ImGui::Unindent(padding.x);
                if (!visible_indices.empty())
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - item_spacing_y);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::Dummy(ImVec2(0, padding.y));
                ImGui::PopStyleVar();
            }
            ImGui::EndGroup();
        }
    }

    HBox::HBox() : Box() {}

    void HBox::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);

        ImGuiStyle &style = ImGui::GetStyle();
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 padding = padded ? style.WindowPadding : ImVec2(0, 0);

        if (scrollable)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
            ImGuiWindowFlags flags =
                ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings;
            if (padded)
                flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
            ImGui::BeginChild("##hbox_scroll", ImVec2(avail.x, 0), 0, flags);
            ImGui::PopStyleVar();
            avail = ImGui::GetContentRegionAvail();
        }
        else
        {
            ImGui::BeginGroup();
            if (padded)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::Dummy(ImVec2(0, padding.y));
                ImGui::PopStyleVar();
                ImGui::Indent(padding.x);
            }
            avail.x -= padding.x * 2.0f;
            avail.y -= padding.y * 2.0f;
            avail.x = std::max(0.0f, avail.x);
            avail.y = std::max(0.0f, avail.y);
        }

        ImVec2 original_spacing = style.ItemSpacing;
        float item_spacing_x = (spacing >= 0.0f) ? spacing : original_spacing.x;

        // Determine which children are visible. This is done every frame to catch
        // dynamic changes in child visibility.
        std::vector<size_t> current_visible_indices;
        current_visible_indices.reserve(children.size());
        for (size_t i = 0; i < children.size(); ++i)
        {
            if (children[i].control->isVisible())
                current_visible_indices.push_back(i);
        }

        bool auto_size_changed = false;
        for (size_t i : m_cachedVisibleIndices)
        {
            auto &child = children[i];
            if (child.sizing.mode == Sizing::Mode::Auto)
            {
                if (std::abs(child.lastKnownSize.x - child.sizeUsedInLastLayout.x) > 0.5f)
                {
                    auto_size_changed = true;
                    break;
                }
            }
        }

        if (m_layoutDirty || std::abs(avail.x - m_lastAvail.x) > 0.5f || current_visible_indices != m_cachedVisibleIndices || auto_size_changed)
        {
            m_cachedVisibleIndices = current_visible_indices;
            m_cachedTotalStretchWeight = 0.0f;
            m_cachedFixedSize = 0.0f;

            float total_spacing = 0.0f;
            if (m_cachedVisibleIndices.size() > 1)
                total_spacing = (m_cachedVisibleIndices.size() - 1) * item_spacing_x;

            for (size_t i : m_cachedVisibleIndices)
            {
                auto &child = children[i];
                switch (child.sizing.mode)
                {
                case Sizing::Mode::Stretch:
                    m_cachedTotalStretchWeight += child.sizing.value;
                    break;
                case Sizing::Mode::Fixed:
                    m_cachedFixedSize += child.sizing.value;
                    break;
                case Sizing::Mode::Percent:
                    m_cachedFixedSize += avail.x * child.sizing.value;
                    break;
                case Sizing::Mode::Auto:
                    m_cachedFixedSize += child.lastKnownSize.x;
                    child.sizeUsedInLastLayout.x = child.lastKnownSize.x;
                    break;
                }
            }

            m_cachedStretchUnit = 0.0f;
            if (m_cachedTotalStretchWeight > 0)
            {
                float remaining_width = avail.x - m_cachedFixedSize - total_spacing;
                if (!scrollable)
                    remaining_width = std::max(0.0f, remaining_width);
                m_cachedStretchUnit = remaining_width / m_cachedTotalStretchWeight;
            }

            m_layoutDirty = false;
            m_lastAvail = avail;
        }

        const auto &visible_indices = m_cachedVisibleIndices;

        for (size_t idx = 0; idx < visible_indices.size(); ++idx)
        {
            size_t i = visible_indices[idx];
            auto &child = children[i];

            float child_width = 0.0f;
            bool use_child_window = false;

            switch (child.sizing.mode)
            {
            case Sizing::Mode::Stretch:
                child_width = m_cachedStretchUnit * child.sizing.value;
                if (!scrollable)
                    child_width = std::max(0.0f, child_width);
                use_child_window = true;
                break;
            case Sizing::Mode::Fixed:
                child_width = child.sizing.value;
                use_child_window = true;
                break;
            case Sizing::Mode::Percent:
                child_width = avail.x * child.sizing.value;
                use_child_window = true;
                break;
            case Sizing::Mode::Auto:
                break;
            }

            ImGui::PushID(static_cast<int>(i));

            if (use_child_window)
            {
                ImGuiChildFlags flags = fillHeight ? 0 : ImGuiChildFlags_AutoResizeY;
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings;
                if (!showChildScrollbars)
                    window_flags |= ImGuiWindowFlags_NoScrollbar;
                ImGui::BeginChild("##box_item",
                                  ImVec2(child_width, fillHeight ? avail.y : 0), flags,
                                  window_flags);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, original_spacing);
                ImGui::PushItemWidth(-1.0f);

                child.control->render();

                ImGui::PopItemWidth();
                ImGui::PopStyleVar();
                ImGui::EndChild();
            }
            else
            {
                ImGui::BeginGroup();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, original_spacing);

                child.control->render();

                ImGui::PopStyleVar();
                ImGui::EndGroup();
            }

            child.lastKnownSize = ImGui::GetItemRectSize();
            ImGui::PopID();

            if (idx < visible_indices.size() - 1)
            {
                ImGui::SameLine(0.0f, item_spacing_x);
            }
        }

        if (scrollable && autoScroll &&
            ImGui::GetScrollX() >= ImGui::GetScrollMaxX())
        {
            ImGui::SetScrollHereX(1.0f);
        }

        if (scrollable)
        {
            ImGui::EndChild();
        }
        else
        {
            if (padded)
            {
                ImGui::Unindent(padding.x);
                if (!visible_indices.empty())
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - original_spacing.y);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::Dummy(ImVec2(0, padding.y));
                ImGui::PopStyleVar();
            }
            ImGui::EndGroup();
        }
    }

    FlowBox::FlowBox() : Box() {}

    void FlowBox::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);

        ImGuiStyle &style = ImGui::GetStyle();
        float contentWidth = ImGui::GetContentRegionAvail().x;
        ImVec2 padding = padded ? style.WindowPadding : ImVec2(0, 0);

        if (scrollable)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
            if (padded)
                flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
            ImGui::BeginChild(
                "##flowbox_scroll",
                ImVec2(0, fillHeight ? ImGui::GetContentRegionAvail().y : 0), 0, flags);
            ImGui::PopStyleVar();
            contentWidth = ImGui::GetContentRegionAvail().x;
        }
        else
        {
            ImGui::BeginGroup();
            if (padded)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::Dummy(ImVec2(0, padding.y));
                ImGui::PopStyleVar();
                ImGui::Indent(padding.x);
                contentWidth -= padding.x * 2.0f;
                contentWidth = std::max(0.0f, contentWidth);
            }
        }

        if (flow_data.size() != children.size())
            flow_data.resize(children.size());

        ImVec2 original_spacing = style.ItemSpacing;
        float item_spacing_x = (spacing >= 0.0f) ? spacing : original_spacing.x;
        float item_spacing_y = (spacing >= 0.0f) ? spacing : original_spacing.y;

        // Determine which children are visible. This is done every frame to catch
        // dynamic changes in child visibility.
        std::vector<size_t> current_visible_indices;
        current_visible_indices.reserve(children.size());
        for (size_t i = 0; i < children.size(); ++i)
        {
            if (children[i].control->isVisible())
                current_visible_indices.push_back(i);
        }

        if (m_layoutDirty || std::abs(contentWidth - m_lastAvail.x) > 0.5f || current_visible_indices != m_cachedVisibleIndices)
        {
            m_cachedVisibleIndices = current_visible_indices;
            m_layoutDirty = false;
            m_lastAvail.x = contentWidth;
        }

        const auto &visible_indices = m_cachedVisibleIndices;

        std::vector<std::vector<size_t>> lines;
        if (!visible_indices.empty())
        {
            lines.emplace_back();
            float current_line_width = 0.0f;
            for (size_t i : visible_indices)
            {
                auto &child = children[i];
                float item_width = 0.0f;

                // Predict rigid widths to prevent frame-feedback loops from stretched
                // items pushing themselves onto new lines
                if (child.sizing.mode == Sizing::Mode::Fixed)
                    item_width = child.sizing.value;
                else if (child.sizing.mode == Sizing::Mode::Percent)
                    item_width = contentWidth * child.sizing.value;
                else if (child.sizing.mode == Sizing::Mode::Stretch)
                    item_width = 10.0f; // Minimal basis size for a stretchable item
                else
                    item_width = flow_data[i].lastKnownWidth > 0
                                     ? flow_data[i].lastKnownWidth
                                     : 50.0f;

                float curr_spacing = lines.back().empty() ? 0.0f : item_spacing_x;

                if (!lines.back().empty() &&
                    current_line_width + curr_spacing + item_width >
                        contentWidth + 0.5f)
                {
                    lines.emplace_back();
                    current_line_width = 0.0f;
                    curr_spacing = 0.0f;
                }

                lines.back().push_back(i);
                current_line_width += curr_spacing + item_width;
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(item_spacing_x, item_spacing_y));

        for (size_t l = 0; l < lines.size(); ++l)
        {
            const auto &line = lines[l];
            float fixed_line_width = 0.0f;
            float total_stretch_weight = 0.0f;

            for (size_t i = 0; i < line.size(); ++i)
            {
                auto &child = children[line[i]];
                if (child.sizing.mode == Sizing::Mode::Stretch)
                    total_stretch_weight += child.sizing.value;
                else if (child.sizing.mode == Sizing::Mode::Fixed)
                    fixed_line_width += child.sizing.value;
                else if (child.sizing.mode == Sizing::Mode::Percent)
                    fixed_line_width += contentWidth * child.sizing.value;
                else
                    fixed_line_width += flow_data[line[i]].lastKnownWidth;
            }
            fixed_line_width +=
                item_spacing_x * (line.size() > 1 ? line.size() - 1 : 0);

            float remaining_space = std::max(0.0f, contentWidth - fixed_line_width);
            float x_spacing = item_spacing_x;
            float offset = 0.0f;

            if (m_align == Align::Right)
                offset = remaining_space;
            else if (m_align == Align::Center)
                offset = remaining_space / 2.0f;
            else if (m_align == Align::Justify && line.size() > 1 &&
                     total_stretch_weight == 0)
                x_spacing = item_spacing_x + remaining_space / (line.size() - 1);

            if (offset > 0)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

            for (size_t i = 0; i < line.size(); ++i)
            {
                size_t child_idx = line[i];
                auto &child = children[child_idx];

                float child_width = 0.0f;
                bool use_child_window = false;

                switch (child.sizing.mode)
                {
                case Sizing::Mode::Stretch:
                    child_width =
                        (remaining_space / total_stretch_weight) * child.sizing.value;
                    use_child_window = true;
                    break;
                case Sizing::Mode::Fixed:
                    child_width = child.sizing.value;
                    use_child_window = true;
                    break;
                case Sizing::Mode::Percent:
                    child_width = contentWidth * child.sizing.value;
                    use_child_window = true;
                    break;
                case Sizing::Mode::Auto:
                    break;
                }

                ImGui::PushID(static_cast<int>(child_idx));

                if (use_child_window)
                {
                    ImGuiChildFlags flags = fillHeight ? 0 : ImGuiChildFlags_AutoResizeY;
                    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings;
                    if (!showChildScrollbars)
                        window_flags |= ImGuiWindowFlags_NoScrollbar;
                    ImGui::BeginChild(
                        "##flow_item",
                        ImVec2(child_width,
                               fillHeight ? ImGui::GetContentRegionAvail().y : 0),
                        flags, window_flags);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, original_spacing);
                    ImGui::PushItemWidth(-1.0f);

                    child.control->render();

                    ImGui::PopItemWidth();
                    ImGui::PopStyleVar();
                    ImGui::EndChild();
                }
                else
                {
                    ImGui::BeginGroup();
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, original_spacing);

                    child.control->render();

                    ImGui::PopStyleVar();
                    ImGui::EndGroup();
                }

                float actual_width = ImGui::GetItemRectSize().x;
                if (std::abs(flow_data[child_idx].lastKnownWidth - actual_width) > 1.0f)
                {
                    flow_data[child_idx].lastKnownWidth = actual_width;
                }

                ImGui::PopID();

                if (i < line.size() - 1)
                    ImGui::SameLine(0.0f, x_spacing);
            }
        }

        ImGui::PopStyleVar();

        if (scrollable)
        {
            ImGui::EndChild();
        }
        else
        {
            if (padded)
            {
                ImGui::Unindent(padding.x);
                if (!visible_indices.empty())
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - item_spacing_y);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::Dummy(ImVec2(0, padding.y));
                ImGui::PopStyleVar();
            }
            ImGui::EndGroup();
        }
    }
} // namespace mui