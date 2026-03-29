#include "box.hpp"
#include "../core/scoped.hpp"
#include <cmath>
#include <algorithm>
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

    std::unique_ptr<ScopedStyle> scrollable_padding;
    if (scrollable)
    {
      ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
      scrollable_padding = std::make_unique<ScopedStyle>(ImGuiStyleVar_WindowPadding, padding);
      if (padded)
        flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
      ImGui::BeginChild("##vbox_scroll", ImVec2(0, avail.y), 0, flags);
      avail = ImGui::GetContentRegionAvail();
    }
    else
    {
      ImGui::BeginGroup();
      if (padded)
      {
        {
          ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
          ImGui::Dummy(ImVec2(0, padding.y));
        }
        ImGui::Indent(padding.x);
      }
      avail.x -= padding.x * 2.0f;
      avail.y -= padding.y * 2.0f;
    }

    ImVec2 original_spacing = style.ItemSpacing;
    float item_spacing_y = (spacing >= 0.0f) ? spacing : original_spacing.y;

    {
      ScopedStyle main_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(original_spacing.x, item_spacing_y));

      float total_stretch_weight = 0.0f;
      float fixed_height = 0.0f;
      float total_spacing = 0.0f;
      if (children.size() > 1)
        total_spacing = (children.size() - 1) * item_spacing_y;

      // Calculate how much space is consumed by non-stretchy items
      for (auto &child : children)
      {
        switch (child.sizing.mode)
        {
        case Sizing::Mode::Stretch:
          total_stretch_weight += child.sizing.value;
          break;
        case Sizing::Mode::Fixed:
          fixed_height += child.sizing.value;
          break;
        case Sizing::Mode::Percent:
          fixed_height += avail.y * child.sizing.value;
          break;
        case Sizing::Mode::Auto:
          fixed_height += child.lastKnownSize.y;
          break;
        }
      }

      // Distribute remaining space amongst stretchy children
      float stretch_unit_height = 0.0f;
      if (total_stretch_weight > 0)
      {
        float remaining_height = avail.y - fixed_height - total_spacing;
        if (!scrollable)
          remaining_height = std::max(0.0f, remaining_height);
        stretch_unit_height = remaining_height / total_stretch_weight;
      }

      for (size_t i = 0; i < children.size(); ++i)
      {
        auto &child = children[i];

        float child_height = 0.0f;
        bool use_child_window = false;

        switch (child.sizing.mode)
        {
        case Sizing::Mode::Stretch:
          child_height = stretch_unit_height * child.sizing.value;
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

        if (use_child_window)
        {
          ImGui::BeginChild(ImGui::GetID((void *)(intptr_t)i), ImVec2(0, child_height), 0, ImGuiWindowFlags_NoSavedSettings);
          ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
          ScopedItemWidth width(-1); // Stretch children fill width
          child.control->render();
          ImGui::EndChild();
        }
        else // Auto
        {
          ScopedID id_scope;
          if (child.control->getID().empty())
            id_scope.push(static_cast<int>(i));
          ImGui::BeginGroup();
          ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
          ScopedItemWidth width;
          if (fillWidth)
            width.push(-1.0f);
          child.control->render();
          ImGui::EndGroup();
        }
        child.lastKnownSize = ImGui::GetItemRectSize();
      }
      
      if (scrollable && autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      {
        ImGui::SetScrollHereY(1.0f);
      }
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
        {
          ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
          ImGui::Dummy(ImVec2(0, padding.y));
        }
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

    std::unique_ptr<ScopedStyle> scrollable_padding;
    if (scrollable)
    {
      ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings;
      scrollable_padding = std::make_unique<ScopedStyle>(ImGuiStyleVar_WindowPadding, padding);
      if (padded)
        flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
      ImGui::BeginChild("##hbox_scroll", ImVec2(avail.x, 0), 0, flags);
      avail = ImGui::GetContentRegionAvail();
    }
    else
    {
      ImGui::BeginGroup();
      if (padded)
      {
        {
          ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
          ImGui::Dummy(ImVec2(0, padding.y));
        }
        ImGui::Indent(padding.x);
      }
      avail.x -= padding.x * 2.0f;
      avail.y -= padding.y * 2.0f;
    }

    ImVec2 original_spacing = style.ItemSpacing;
    float item_spacing_x = (spacing >= 0.0f) ? spacing : original_spacing.x;

    {
      // ScopedStyle main_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(item_spacing_x, original_spacing.y));

      int stretch_count = 0;
      float fixed_width = 0.0f;
      float total_spacing = 0.0f;
      if (children.size() > 1)
        total_spacing = (children.size() - 1) * item_spacing_x;

      // Calculate how much space is consumed by non-stretchy items
      for (auto &child : children)
      {
        switch (child.sizing.mode)
        {
        case Sizing::Mode::Stretch:
          stretch_count++;
          break;
        case Sizing::Mode::Fixed:
          fixed_width += child.sizing.value;
          break;
        case Sizing::Mode::Percent:
          fixed_width += avail.x * child.sizing.value;
          break;
        case Sizing::Mode::Auto:
          fixed_width += child.lastKnownSize.x;
          break;
        }
      }

      // Distribute remaining space amongst stretchy children
      float stretch_width = 0.0f;
      if (stretch_count > 0)
      {
        float remaining_width = avail.x - fixed_width - total_spacing;
        if (!scrollable)
          remaining_width = std::max(0.0f, remaining_width);
        stretch_width = remaining_width / stretch_count;
      }

      for (size_t i = 0; i < children.size(); ++i)
      {
        auto &child = children[i];

        float child_width = 0.0f;
        bool use_child_window = false;

        switch (child.sizing.mode)
        {
        case Sizing::Mode::Stretch:
          child_width = stretch_width * child.sizing.value;
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

        if (use_child_window)
        {
          ImGuiChildFlags flags = fillHeight ? 0 : ImGuiChildFlags_AutoResizeY;
          ImGui::BeginChild(ImGui::GetID((void *)(intptr_t)i), ImVec2(child_width, fillHeight ? avail.y : 0), flags, ImGuiWindowFlags_NoSavedSettings);
          ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
          ScopedItemWidth width(-1); // Stretch children fill width of their child window
          child.control->render();
          ImGui::EndChild();
        }
        else // Auto
        {
          ScopedID id_scope;
          if (child.control->getID().empty())
            id_scope.push(static_cast<int>(i));
          ImGui::BeginGroup();
          ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
          child.control->render();
          ImGui::EndGroup();
        }
        child.lastKnownSize = ImGui::GetItemRectSize();

        if (i < children.size() - 1)
        {
          ImGui::SameLine(0, item_spacing_x);
        }
      }

      if (scrollable && autoScroll && ImGui::GetScrollX() >= ImGui::GetScrollMaxX())
      {
        ImGui::SetScrollHereX(1.0f);
      }
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
        {
          // This dummy widget is only for adding the bottom padding inside the group.
          // The horizontal padding is handled by the Indent/Unindent pair.
          ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
          ImGui::Dummy(ImVec2(0, padding.y));
        }
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

    std::unique_ptr<ScopedStyle> scrollable_padding;
    if (scrollable)
    {
      ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
      scrollable_padding = std::make_unique<ScopedStyle>(ImGuiStyleVar_WindowPadding, padding);
      if (padded)
        flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
      ImGui::BeginChild("##flowbox_scroll", ImVec2(0, fillHeight ? ImGui::GetContentRegionAvail().y : 0), 0, flags);
      contentWidth = ImGui::GetContentRegionAvail().x;
    }
    else
    {
      ImGui::BeginGroup();
      if (padded)
      {
        {
          ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
          ImGui::Dummy(ImVec2(0, padding.y));
        }
        ImGui::Indent(padding.x);
        contentWidth -= padding.x * 2.0f;
      }
    }

    if (flow_data.size() != children.size())
      flow_data.resize(children.size());

    ImVec2 original_spacing = style.ItemSpacing;
    float item_spacing_x = original_spacing.x;
    float item_spacing_y = original_spacing.y;

    // --- Step 1: Group items into lines based on cached widths ---
    std::vector<std::vector<size_t>> lines;
    if (!children.empty())
    {
      lines.emplace_back();
      float current_line_width = 0.0f;
      for (size_t i = 0; i < children.size(); ++i)
      {
        const auto &child = children[i];
        float item_width = 0.0f;
        switch (child.sizing.mode)
        {
        case Sizing::Mode::Fixed:
          item_width = child.sizing.value;
          break;
        case Sizing::Mode::Percent:
          item_width = contentWidth * child.sizing.value;
          break;
        case Sizing::Mode::Auto:
        case Sizing::Mode::Stretch: // For line breaking, treat stretch as auto
        default:
          item_width = flow_data[i].lastKnownWidth;
          if (item_width == 0)
            item_width = 50.0f; // Guess for new items
          break;
        }

        float spacing = lines.back().empty() ? 0.0f : item_spacing_x;

        if (!lines.back().empty() &&
            current_line_width + spacing + item_width > contentWidth + 0.5f)
        {
          // Doesn't fit, start a new line
          lines.emplace_back();
          current_line_width = 0.0f;
          spacing = 0.0f;
        }

        lines.back().push_back(i);
        current_line_width += spacing + item_width;
      }
    }

    ScopedStyle main_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(item_spacing_x, item_spacing_y));

    // --- Step 2: Render each line with alignment ---
    for (size_t l = 0; l < lines.size(); ++l)
    {
      const auto &line = lines[l];
      float line_width_non_stretch = 0.0f;
      float total_stretch_weight = 0.0f;

      for (size_t idx : line)
      {
        const auto &child = children[idx];
        if (child.sizing.mode == Sizing::Mode::Stretch)
        {
          total_stretch_weight += child.sizing.value;
        }
        else
        {
          float item_width = 0.0f;
          switch (child.sizing.mode)
          {
          case Sizing::Mode::Fixed:
            item_width = child.sizing.value;
            break;
          case Sizing::Mode::Percent:
            item_width = contentWidth * child.sizing.value;
            break;
          case Sizing::Mode::Auto:
          default:
            item_width = flow_data[idx].lastKnownWidth;
            if (item_width == 0)
              item_width = 50.0f;
            break;
          }
          line_width_non_stretch += item_width;
        }
      }
      line_width_non_stretch += item_spacing_x * (line.size() > 1 ? line.size() - 1 : 0);

      float remaining_space = contentWidth - line_width_non_stretch;
      float spacing = item_spacing_x;
      float offset = 0.0f;
      float stretch_unit_width = 0.0f;

      if (total_stretch_weight > 0 && remaining_space > 0)
      {
        stretch_unit_width = remaining_space / total_stretch_weight;
      }
      else
      {
        // Only apply alignment if there are no stretchy items on the line
        if (m_align == Align::Right)
          offset = remaining_space;
        else if (m_align == Align::Center)
          offset = remaining_space / 2.0f;
        else if (m_align == Align::Justify && line.size() > 1)
          spacing = item_spacing_x + remaining_space / (line.size() - 1);
      }

      if (offset > 0)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

      for (size_t i = 0; i < line.size(); ++i)
      {
        size_t child_idx = line[i];
        const auto &child = children[child_idx];

        {
          ScopedItemWidth width_scope;
          ScopedID id_scope;
          if (child.control->getID().empty())
            id_scope.push(static_cast<int>(child_idx));

          bool use_width_scope = false;
          float item_width = 0.0f;
          switch (child.sizing.mode)
          {
          case Sizing::Mode::Stretch:
            item_width = stretch_unit_width * child.sizing.value;
            use_width_scope = true;
            break;
          case Sizing::Mode::Fixed:
            item_width = child.sizing.value;
            use_width_scope = true;
            break;
          case Sizing::Mode::Percent:
            item_width = contentWidth * child.sizing.value;
            use_width_scope = true;
            break;
          default:
            break;
          }
          if (use_width_scope)
          {
            width_scope.push(item_width);
          }

          ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
          child.control->render();
        }

        float actual_width = ImGui::GetItemRectSize().x;
        if (std::abs(flow_data[child_idx].lastKnownWidth - actual_width) > 1.0f)
        {
          flow_data[child_idx].lastKnownWidth = actual_width;
        }

        if (i < line.size() - 1)
          ImGui::SameLine(0.0f, spacing);
      }
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
        {
          ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
          ImGui::Dummy(ImVec2(0, padding.y));
        }
      }
      ImGui::EndGroup();
    }
  }

  FlowBoxPtr FlowBox::append(IControlPtr child, bool stretchy) // Keep for compatibility
  {
    this->verifyState();
    children.emplace_back(child, stretchy);
    flow_data.push_back({0, 0, 0, 0});
    return this->self();
  }
} // namespace mui
