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

    if (scrollable)
    {
      ScopedStyle window_padding(ImGuiStyleVar_WindowPadding, padding);
      ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
      if (padded) flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
      ImGui::BeginChild("##vbox_scroll", ImVec2(0, avail.y), 0, flags);
      avail = ImGui::GetContentRegionAvail();
    }
    else
    {
      ImGui::BeginGroup();
      if (padded)
      {
        ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::Dummy(ImVec2(0, padding.y));
        ImGui::Indent(padding.x);
      }
      avail.x -= padding.x * 2.0f;
      avail.y -= padding.y * 2.0f;
    }

    ImVec2 original_spacing = style.ItemSpacing;
    float item_spacing_y = original_spacing.y;

    {
      ScopedStyle main_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(original_spacing.x, item_spacing_y));

      int stretch_count = 0;
      float fixed_height = 0.0f;
      float total_spacing = 0.0f;
      if (children.size() > 1)
        total_spacing = (children.size() - 1) * item_spacing_y;

      // Calculate how much space is consumed by fixed items
      for (auto &child : children)
      {
        if (child.stretchy)
        {
          stretch_count++;
        }
        else
        {
          fixed_height += child.lastKnownSize.y;
        }
      }

      // Distribute remaining space amongst stretchy children
      float stretch_height = 0.0f;
      if (stretch_count > 0)
      {
        float remaining = avail.y - fixed_height - total_spacing;
        stretch_height = std::max(0.0f, remaining / stretch_count);
      }

      for (size_t i = 0; i < children.size(); ++i)
      {
        auto &child = children[i];

        ScopedID id_scope;
        if (child.control->getID().empty())
          id_scope.push(static_cast<int>(i));

        if (child.stretchy)
        {
          // Restrict the stretch element to its calculated proportional height so it fills, but doesn't over-expand
          ImGui::BeginChild(ImGui::GetID((void *)(intptr_t)i), ImVec2(0, stretch_height), 0, ImGuiWindowFlags_NoSavedSettings);
          {
            ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
            ScopedItemWidth width(-1);
            child.control->render();
          }
          ImGui::EndChild();
          child.lastKnownSize.y = stretch_height;
          child.lastKnownSize.x = ImGui::GetItemRectSize().x;
        }
        else
        {
          ImGui::BeginGroup();
          {
            ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
            ScopedItemWidth width;
            if (fillWidth)
              width.push(-1.0f);
            child.control->render();
          }
          ImGui::EndGroup();
          child.lastKnownSize.y = ImGui::GetItemRectSize().y;
          child.lastKnownSize.x = ImGui::GetItemRectSize().x;
        }
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
        ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::Dummy(ImVec2(padding.x * 2.0f, padding.y));
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
      ScopedStyle window_padding(ImGuiStyleVar_WindowPadding, padding);
      ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings;
      if (padded) flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
      ImGui::BeginChild("##hbox_scroll", ImVec2(avail.x, 0), 0, flags);
      avail = ImGui::GetContentRegionAvail();
    }
    else
    {
      ImGui::BeginGroup();
      if (padded)
      {
        ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::Dummy(ImVec2(0, padding.y));
        ImGui::Indent(padding.x);
      }
      avail.x -= padding.x * 2.0f;
      avail.y -= padding.y * 2.0f;
    }

    ImVec2 original_spacing = style.ItemSpacing;
    float item_spacing_x = original_spacing.x;

    {
      //ScopedStyle main_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(item_spacing_x, original_spacing.y));

      int stretch_count = 0;
      float fixed_width = 0.0f;
      float total_spacing = 0.0f;
      if (children.size() > 1)
        total_spacing = (children.size()-1) * item_spacing_x;

      // Calculate how much space is consumed by fixed items
      for (auto &child : children)
      {
        if (child.stretchy)
        {
          stretch_count++;
        }
        else
        {
          fixed_width += child.lastKnownSize.x;
        }
      }

      // Distribute remaining space amongst stretchy children
      float stretch_width = 0.0f;
      if (stretch_count > 0)
      {
        float remaining = avail.x - fixed_width - total_spacing;
        stretch_width = std::max(0.0f, remaining / stretch_count);
      }

      for (size_t i = 0; i < children.size(); ++i)
      {
        auto &child = children[i];

        ScopedID id_scope;
        if (child.control->getID().empty())
          id_scope.push(static_cast<int>(i));

        if (child.stretchy)
        {
          // Restrict the stretch element to its calculated proportional width so it fills, but doesn't over-expand
          ImGuiChildFlags flags = fillHeight ? 0 : ImGuiChildFlags_AutoResizeY;
          ImGui::BeginChild(ImGui::GetID((void *)(intptr_t)i), ImVec2(stretch_width, fillHeight ? avail.y : 0), flags, ImGuiWindowFlags_NoSavedSettings);
          {
            ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
            ScopedItemWidth width(-1);
            child.control->render();
          }
          ImGui::EndChild();
          child.lastKnownSize.x = stretch_width;
          child.lastKnownSize.y = ImGui::GetItemRectSize().y;
        }
        else
        {
          if (fillHeight)
          {
            ImGui::BeginChild(ImGui::GetID((void *)(intptr_t)i), ImVec2(0, avail.y), ImGuiChildFlags_AutoResizeX, ImGuiWindowFlags_NoSavedSettings);
          }
          else
          {
            ImGui::BeginGroup();
          }

          {
            ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
            child.control->render();
          }

          if (fillHeight)
          {
            ImGui::EndChild();
          }
          else
          {
            ImGui::EndGroup();
          }
          child.lastKnownSize.x = ImGui::GetItemRectSize().x;
          child.lastKnownSize.y = ImGui::GetItemRectSize().y;
        }

        if (i < children.size() - 1)
        {
          ImGui::SameLine();
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
        if (!children.empty())
        {
          ImGui::SameLine(0, 0);
          ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
          ImGui::Dummy(ImVec2(padding.x, 0));
        }
        ImGui::Unindent(padding.x);
        ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::Dummy(ImVec2(children.empty() ? padding.x * 2.0f : 0.0f, padding.y));
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
      ScopedStyle window_padding(ImGuiStyleVar_WindowPadding, padding);
      ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
      if (padded) flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
      ImGui::BeginChild("##flowbox_scroll", ImVec2(0, fillHeight ? ImGui::GetContentRegionAvail().y : 0), 0, flags);
      contentWidth = ImGui::GetContentRegionAvail().x;
    }
    else
    {
      ImGui::BeginGroup();
      if (padded)
      {
        ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::Dummy(ImVec2(0, padding.y));
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
        float item_width = flow_data[i].lastKnownWidth;
        if (item_width == 0)
          item_width = 50.0f; // Guess for new items

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
      float line_width = 0.0f;
      for (size_t i = 0; i < line.size(); ++i)
      {
        line_width += flow_data[line[i]].lastKnownWidth;
      }
      line_width += item_spacing_x * (line.size() > 1 ? line.size() - 1 : 0);

      float remaining_space = contentWidth - line_width;
      float spacing = item_spacing_x;
      float offset = 0.0f;
      float extra_width_per_item = 0.0f;

      if (m_align == Align::Right)
        offset = remaining_space;
      else if (m_align == Align::Center)
        offset = remaining_space / 2.0f;
      else if (m_align == Align::Justify && line.size() > 1)
        spacing = item_spacing_x + remaining_space / (line.size() - 1);

      if (offset > 0)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

      for (size_t i = 0; i < line.size(); ++i)
      {
        size_t child_idx = line[i];

        {
          ScopedItemWidth width;
          ScopedID id_scope;
          if (children[child_idx].control->getID().empty())
            id_scope.push(static_cast<int>(child_idx));

          if (extra_width_per_item > 0.0f)
          {
            float item_width = flow_data[child_idx].lastKnownWidth;
            if (item_width == 0)
              item_width = 50.0f; // Guess for new items
            width.push(item_width + extra_width_per_item);
          }

          ScopedStyle child_spacing(ImGuiStyleVar_ItemSpacing, original_spacing);
          children[child_idx].control->render();
        }

        if (extra_width_per_item <= 0.0f)
        {
          float actual_width = ImGui::GetItemRectSize().x;
          if (std::abs(flow_data[child_idx].lastKnownWidth - actual_width) > 1.0f)
          {
            flow_data[child_idx].lastKnownWidth = actual_width;
          }
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
        ScopedStyle zero_spacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::Dummy(ImVec2(padding.x * 2.0f, padding.y));
      }
      ImGui::EndGroup();
    }
  }

  FlowBoxPtr FlowBox::append(IControlPtr child, bool stretchy)
  {
    this->verifyState();
    children.push_back({child, stretchy});
    flow_data.push_back({0, 0, 0, 0});
    return this->self();
  }
} // namespace mui
