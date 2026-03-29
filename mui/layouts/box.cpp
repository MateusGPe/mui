#include "box.hpp"
#include "../core/scoped.hpp"
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

    ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame;
    if (scrollable)
    {
      table_flags |= ImGuiTableFlags_ScrollY;
    }

    if (ImGui::BeginTable("##vbox_layout", 1, table_flags))
    {
      for (size_t i = 0; i < children.size(); ++i)
      {
        auto &child = children[i];
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        bool needsIndexId = child.control->getID().empty();
        if (needsIndexId)
          ImGui::PushID(static_cast<int>(i));

        if (child.stretchy)
        {
          ScopedItemWidth width(-1);
          child.control->render();
        }
        else
        {
          child.control->render();
        }

        if (needsIndexId)
          ImGui::PopID();

        if (padded)
        {
          ImGui::Spacing();
        }
      }
      ImGui::EndTable();
    }

    if (scrollable)
    {
      if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      {
        ImGui::SetScrollHereY(1.0f);
      }
    }
  }

  HBox::HBox() : Box() {}
  void HBox::renderControl()
  {
    if (!visible)
      return;
    ScopedControlID id(this);

    ImGuiTableFlags table_flags = 0;
    if (scrollable)
    {
      table_flags |= ImGuiTableFlags_ScrollX;
    }

    if (ImGui::BeginTable("##hbox_layout", (int)children.size(), table_flags))
    {
      for (size_t i = 0; i < children.size(); ++i)
      {
        ImGuiTableColumnFlags flags = children[i].stretchy
                                          ? ImGuiTableColumnFlags_WidthStretch
                                          : ImGuiTableColumnFlags_WidthFixed;
        ImGui::TableSetupColumn("", flags);
      }

      ImGui::TableNextRow();

      for (size_t i = 0; i < children.size(); ++i)
      {
        ImGui::TableNextColumn();

        bool needsIndexId = children[i].control->getID().empty();
        if (needsIndexId)
          ImGui::PushID(static_cast<int>(i));

        if (children[i].stretchy)
        {
          ScopedItemWidth width(-FLT_MIN);
          children[i].control->render();
        }
        else
        {
          children[i].control->render();
        }

        if (needsIndexId)
          ImGui::PopID();
      }
      ImGui::EndTable();
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

    if (flow_data.size() != children.size())
      flow_data.resize(children.size());

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

        float spacing = lines.back().empty() ? 0.0f : style.ItemSpacing.x;

        // Allow a small overflow tolerance to prevent boundary flickering
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

    // --- Step 2: Render each line with alignment ---
    for (const auto &line : lines)
    {
      float line_width = 0.0f;
      for (size_t i = 0; i < line.size(); ++i)
      {
        line_width += flow_data[line[i]].lastKnownWidth;
      }
      line_width += style.ItemSpacing.x * (line.size() > 1 ? line.size() - 1 : 0);

      float remaining_space = contentWidth - line_width;
      float spacing = style.ItemSpacing.x;
      float offset = 0.0f;
      float extra_width_per_item = 0.0f;

      if (m_align == Align::Right)
        offset = remaining_space;
      else if (m_align == Align::Center)
        offset = remaining_space / 2.0f;
      else if (m_align == Align::Justify && line.size() > 1)
        spacing = style.ItemSpacing.x + remaining_space / (line.size() - 1);
      // else if (m_align == Align::Fill && line.size() > 0 && remaining_space
      // > 1.0f)
      //     extra_width_per_item = remaining_space / line.size();

      if (offset > 0)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

      for (size_t i = 0; i < line.size(); ++i)
      {
        size_t child_idx = line[i];

        {
          ScopedItemWidth width;
          bool needsIndexId = children[child_idx].control->getID().empty();
          if (needsIndexId)
            ImGui::PushID(static_cast<int>(child_idx));

          if (extra_width_per_item > 0.0f)
          {
            float item_width = flow_data[child_idx].lastKnownWidth;
            if (item_width == 0)
              item_width = 50.0f; // Guess for new items
            width.push(item_width + extra_width_per_item);
          }
          children[child_idx].control->render();

          if (needsIndexId)
            ImGui::PopID();
        }

        if (extra_width_per_item <= 0.0f)
        {
          float actual_width = ImGui::GetItemRectSize().x;
          // Only update if difference is significant to prevent sub-pixel
          // rounding loops
          if (std::abs(flow_data[child_idx].lastKnownWidth - actual_width) >
              1.0f)
          {
            flow_data[child_idx].lastKnownWidth = actual_width;
          }
        }

        if (i < line.size() - 1)
          ImGui::SameLine(0.0f, spacing);
      }
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
