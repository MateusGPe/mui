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
        inlineShadowEnabled =
            false; // PropertyGrid typically doesn't have its own shadow
                   // Initialize new members to default/sentinel values
                   // nameColumnWidth, valueColumnWeight, itemSpacingY,
                   // categorySpacingY, itemIndent are already initialized in .hpp or
                   // have meaningful defaults.
    }

    void PropertyGrid::renderControl()
    {
        if (!(m_flags & PropertyGridFlags_Visible))
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!(m_flags & PropertyGridFlags_Enabled));

        // Standard JUCE/Inspector style grid flags
        ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV |
                                ImGuiTableFlags_BordersOuterH |
                                ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_NoBordersInBody;
        if (m_flags & PropertyGridFlags_ZebraStripes)
        {
            flags |= ImGuiTableFlags_RowBg;
        }

        if (spanAvailWidth)
            flags |= ImGuiTableFlags_SizingStretchProp;

        // Apply custom vertical spacing if set
        ScopedStyle itemSpacingOverride;
        if (itemSpacingY >= 0)
        {
            itemSpacingOverride.push(
                ImGuiStyleVar_ItemSpacing,
                ImVec2(ImGui::GetStyle().ItemSpacing.x, itemSpacingY));
        }

        if (ImGui::BeginTable("##property_grid", 2, flags))
        {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed,
                                    nameColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch,
                                    valueColumnWeight);

            for (size_t c = 0; c < categories.size(); ++c)
            {
                auto &cat = categories[c];
                bool nodeOpen = true;

                // Apply custom vertical spacing between categories if set
                if (c > 0 && categorySpacingY >= 0)
                {
                    ImGui::Dummy(ImVec2(0, categorySpacingY));
                }

                // Render Category Header (if it has a name)
                if (!cat.name.empty())
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    ImGuiTreeNodeFlags treeFlags = cat.flags;
                    if (m_flags & PropertyGridFlags_SpanCategoryHeaders)
                    {
                        treeFlags |= ImGuiTreeNodeFlags_SpanAllColumns;
                    }
                    else
                    {
                        treeFlags |= ImGuiTreeNodeFlags_SpanFullWidth;
                    }

                    ScopedColor headerColorOverride;
                    if (cat.useCustomColor)
                    {
                        headerColorOverride.push(ImGuiCol_Text, cat.color);
                    }

                    ScopedID catId(static_cast<int>(c));
                    std::string headerLabel =
                        cat.icon.empty() ? cat.name : cat.icon + " " + cat.name;
                    nodeOpen = ImGui::TreeNodeEx(headerLabel.c_str(), treeFlags);
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
                        {
                            ImGui::Indent(itemIndent >= 0 ? itemIndent
                                                          : ImGui::GetTreeNodeToLabelSpacing());
                        }

                        ScopedColor labelColorOverride;
                        if (item.useLabelColor)
                        {
                            labelColorOverride.push(ImGuiCol_Text, item.labelColor);
                        }
                        ImGui::TextUnformatted(
                            item.label.c_str()); // Use TextUnformatted for raw string

                        if (!cat.name.empty())
                        {
                            ImGui::Unindent(itemIndent >= 0
                                                ? itemIndent
                                                : ImGui::GetTreeNodeToLabelSpacing());
                        }

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
                                              const std::string &icon, ImVec4 color,
                                              ImGuiTreeNodeFlags flags)
    {
        verifyState();
        PropertyCategory newCat;
        newCat.name = name;
        newCat.icon = icon;
        newCat.flags = flags;
        if (color.w > 0)
        { // Check if color is explicitly set (alpha > 0)
            newCat.color = color;
            newCat.useCustomColor = true;
        }
        categories.push_back(std::move(newCat));
        return self();
    }

    PropertyGridPtr PropertyGrid::addProperty(const std::string &label,
                                              IControlPtr editor,
                                              ImVec4 labelColor)
    {
        verifyState();
        if (categories.empty())
        {
            // If no category was explicitly created, create a blank, headerless group.
            categories.push_back({"",
                                  "",
                                  ImVec4(0, 0, 0, 0),
                                  false,
                                  {},
                                  ImGuiTreeNodeFlags_DefaultOpen});
        }

        // Ensure the editor knows to use all available container width
        // (assuming the target widget respects spanAvailWidth or uses -FLT_MIN
        // context sizing)
        PropertyItem newItem;
        newItem.label = label;
        newItem.editor = editor;
        if (labelColor.w > 0)
        { // Check if color is explicitly set (alpha > 0)
            newItem.labelColor = labelColor;
            newItem.useLabelColor = true;
        }
        categories.back().items.push_back(std::move(newItem));

        return self();
    }

    PropertyGridPtr PropertyGrid::setNameColumnWidth(float width)
    {
        verifyState();
        nameColumnWidth = width;
        return self();
    }

    PropertyGridPtr PropertyGrid::setValueColumnWeight(float weight)
    {
        verifyState();
        valueColumnWeight = weight;
        return self();
    }
    PropertyGridPtr PropertyGrid::setItemSpacingY(float spacing)
    {
        verifyState();
        itemSpacingY = spacing;
        return self();
    }
    PropertyGridPtr PropertyGrid::setCategorySpacingY(float spacing)
    {
        verifyState();
        categorySpacingY = spacing;
        return self();
    }
    PropertyGridPtr PropertyGrid::setItemIndent(float indent)
    {
        verifyState();
        itemIndent = indent;
        return self();
    }

    PropertyGridPtr PropertyGrid::setVisible(bool v)
    {
        verifyState();
        if (v)
            m_flags |= PropertyGridFlags_Visible;
        else
            m_flags &= ~PropertyGridFlags_Visible;
        Control::setVisible(v);
        return self();
    }

    PropertyGridPtr PropertyGrid::setEnabled(bool e)
    {
        verifyState();
        if (e)
            m_flags |= PropertyGridFlags_Enabled;
        else
            m_flags &= ~PropertyGridFlags_Enabled;
        Control::setEnabled(e);
        return self();
    }

    PropertyGridPtr PropertyGrid::setSpanCategoryHeaders(bool span)
    {
        verifyState();
        if (span)
            m_flags |= PropertyGridFlags_SpanCategoryHeaders;
        else
            m_flags &= ~PropertyGridFlags_SpanCategoryHeaders;
        return self();
    }

    PropertyGridPtr PropertyGrid::setZebraStripes(bool show)
    {
        verifyState();
        if (show)
            m_flags |= PropertyGridFlags_ZebraStripes;
        else
            m_flags &= ~PropertyGridFlags_ZebraStripes;
        return self();
    }
} // namespace mui