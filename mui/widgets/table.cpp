// widgets/table.cpp
#include "table.hpp"
#include "../core/app.hpp"
#include <imgui.h>

namespace mui
{
    Table::Table() { App::assertMainThread(); }

    void Table::renderControl()
    {
        if (!visible || columns.empty()) return;
        ImGui::PushID(this);

        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
        if (sortable) flags |= ImGuiTableFlags_Sortable;
        if (spanAvailWidth) flags |= ImGuiTableFlags_SizingStretchProp;

        ImVec2 size = ImVec2(spanAvailWidth ? -FLT_MIN : 0, 0);

        if (ImGui::BeginTable("##table", (int)columns.size(), flags, size))
        {
            ImGui::TableSetupScrollFreeze(0, 1); // Freeze header
            for (const auto &col : columns)
            {
                ImGuiTableColumnFlags colFlags = col.fixedWidth ? ImGuiTableColumnFlags_WidthFixed : ImGuiTableColumnFlags_WidthStretch;
                ImGui::TableSetupColumn(col.name.c_str(), colFlags, col.weight);
            }
            ImGui::TableHeadersRow();

            // Handle Sorting
            if (sortable)
            {
                if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
                {
                    if (sorts_specs->SpecsDirty && onSortRequestedCb)
                    {
                        onSortRequestedCb(sorts_specs->Specs->ColumnIndex, sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending);
                        sorts_specs->SpecsDirty = false;
                    }
                }
            }

            // Render Rows
            for (size_t r = 0; r < rows.size(); ++r)
            {
                ImGui::TableNextRow();
                for (size_t c = 0; c < columns.size(); ++c)
                {
                    ImGui::TableSetColumnIndex(c);
                    
                    // Allow row selection on the first column to capture the whole row
                    if (c == 0) {
                        std::string selId = "##row_sel_" + std::to_string(r);
                        
                        // FIX: Changed ImGuiSelectableFlags_AllowItemOverlap to ImGuiSelectableFlags_AllowOverlap
                        if (ImGui::Selectable(selId.c_str(), selectedRow == (int)r, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                            selectedRow = (int)r;
                            if (onRowSelectedCb) onRowSelectedCb(selectedRow);
                        }
                        ImGui::SameLine();
                    }

                    if (c < rows[r].size() && rows[r][c]) {
                        // Ensure widgets fit in cell
                        ImGui::PushItemWidth(-FLT_MIN);
                        rows[r][c]->render();
                        ImGui::PopItemWidth();
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::PopID();
    }

    TablePtr Table::addColumn(const std::string &name, float weight, bool fixedWidth) { columns.push_back({name, weight, fixedWidth}); return self(); }
    TablePtr Table::addRow(const std::vector<ControlPtr> &rowItems) { rows.push_back(rowItems); return self(); }
    TablePtr Table::clearRows() { rows.clear(); selectedRow = -1; return self(); }
    int Table::getSelectedRow() const { return selectedRow; }
    TablePtr Table::setSelectedRow(int index) { selectedRow = index; return self(); }
    TablePtr Table::setSortable(bool s) { sortable = s; return self(); }
    TablePtr Table::onRowSelected(std::function<void(int)> cb) { onRowSelectedCb = std::move(cb); return self(); }
    TablePtr Table::onSortRequested(std::function<void(int, bool)> cb) { onSortRequestedCb = std::move(cb); return self(); }
} // namespace mui
