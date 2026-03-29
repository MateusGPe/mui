// widgets/propertygrid.cpp
#include "propertygrid.hpp"
#include "../core/app.hpp"
#include "../core/scoped.hpp"
#include <imgui.h>

namespace mui
{
  PropertyGrid::PropertyGrid()
  {
    App::assertMainThread();
    inlineShadowEnabled = false;
  }

  void PropertyGrid::renderControl()
  {
    if (!visible)
      return;
    ScopedControlID id(this);
    ImGui::BeginDisabled(!enabled);

    // Standard JUCE/Inspector style grid flags
    ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV |
                            ImGuiTableFlags_BordersOuterH |
                            ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_NoBordersInBody;

    if (spanAvailWidth)
      flags |= ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("##property_grid", 2, flags))
    {
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed,
                              nameColumnWidth);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

      for (size_t c = 0; c < categories.size(); ++c)
      {
        auto &cat = categories[c];
        bool nodeOpen = true;

        // Render Category Header (if it has a name)
        if (!cat.name.empty())
        {
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);

          ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_SpanFullWidth;
          if (cat.defaultOpen)
            treeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

          ScopedID catId(static_cast<int>(c));
          nodeOpen = ImGui::TreeNodeEx(cat.name.c_str(), treeFlags);
        }

        // Render Items inside this category
        if (nodeOpen)
        {
          for (size_t i = 0; i < cat.items.size(); ++i)
          {
            auto &item = cat.items[i];
            ImGui::TableNextRow();

            // Column 0: Label
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();

            // Indent the text so it aligns nicely under the category arrow
            if (!cat.name.empty())
              ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());

            ImGui::TextUnformatted(item.label.c_str());

            if (!cat.name.empty())
              ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

            // Column 1: Editor Widget
            ImGui::TableSetColumnIndex(1);
            if (item.editor)
            {
              bool needsIndexId = item.editor->getID().empty();
              if (needsIndexId)
                ImGui::PushID(static_cast<int>(i));

              // Force the editor widget to fill the column width
              ScopedItemWidth width(-FLT_MIN);

              // Prevent standard widgets from clipping/wrapping weirdly
              // by informing the child control to span available width
              item.editor->render();

              if (needsIndexId)
                ImGui::PopID();
            }
          }

          if (!cat.name.empty())
          {
            ImGui::TreePop();
          }
        }
      }
      ImGui::EndTable();
    }

    renderTooltip();
    ImGui::EndDisabled();
  }

  PropertyGridPtr PropertyGrid::addCategory(const std::string &name,
                                            bool defaultOpen)
  {
    verifyState();
    categories.push_back({name, {}, defaultOpen});
    return self();
  }

  PropertyGridPtr PropertyGrid::addProperty(const std::string &label,
                                            IControlPtr editor)
  {
    verifyState();
    if (categories.empty())
    {
      // If no category was explicitly created, create a blank, headerless group.
      categories.push_back({"", {}, true});
    }

    // Ensure the editor knows to use all available container width
    // (assuming the target widget respects spanAvailWidth or uses -FLT_MIN
    // context sizing)
    categories.back().items.push_back({label, editor});

    return self();
  }

  PropertyGridPtr PropertyGrid::setNameColumnWidth(float width)
  {
    verifyState();
    nameColumnWidth = width;
    return self();
  }
} // namespace mui