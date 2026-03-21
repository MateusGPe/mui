// widgets/table.cpp
#include "table.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    Table::Table() { App::assertMainThread(); }

    void Table::renderControl()
    {
        if (!visible || columns.empty())
            return;
        ScopedID id(this);

        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
        if (sortable)
            flags |= ImGuiTableFlags_Sortable;
        if (spanAvailWidth)
            flags |= ImGuiTableFlags_SizingStretchProp;

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
                if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs())
                {
                    if (sorts_specs->SpecsDirty)
                    {
                        onSortRequestedSignal(sorts_specs->Specs->ColumnIndex, sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending);
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
                    if (c == 0)
                    {
                        std::string selId = "##row_sel_" + std::to_string(r);

                        // FIX: Changed ImGuiSelectableFlags_AllowItemOverlap to ImGuiSelectableFlags_AllowOverlap
                        if (ImGui::Selectable(selId.c_str(), selectedRow == (int)r, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
                        {
                            selectedRow = (int)r;
                            onRowSelectedSignal(selectedRow);
                            if (ImGui::IsMouseDoubleClicked(0))
                            {
                                onRowDoubleClickedSignal(selectedRow);
                            }
                        }
                        ImGui::SameLine();
                    }

                    if (c < rows[r].size() && rows[r][c])
                    {
                        // Ensure widgets fit in cell
                        ScopedItemWidth width(-FLT_MIN);
                        rows[r][c]->render();
                    }
                }
            }
            ImGui::EndTable();
        }
    }

    TablePtr Table::addColumn(const std::string &name, float weight, bool fixedWidth)
    {
        columns.push_back({name, weight, fixedWidth});
        return self();
    }
    TablePtr Table::addRow(const std::vector<IControlPtr> &rowItems)
    {
        rows.push_back(rowItems);
        return self();
    }
    TablePtr Table::clearRows()
    {
        rows.clear();
        selectedRow = -1;
        return self();
    }
    int Table::getSelectedRow() const { return selectedRow; }
    TablePtr Table::setSelectedRow(int index)
    {
        selectedRow = index;
        return self();
    }

    TablePtr Table::bind(std::shared_ptr<Observable<int>> observable)
    {
        setSelectedRow(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const int &val) {
            mui::App::queueMain([this, val]() { this->setSelectedRow(val); });
        }));
        m_connections.push_back(onRowSelectedSignal.connect([observable](int val) {
            observable->set(val);
        }));
        return self();
    }

    TablePtr Table::setSortable(bool s)
    {
        sortable = s;
        return self();
    }
    TablePtr Table::onRowSelected(std::function<void(int)> cb)
    {
        if (cb) m_connections.push_back(onRowSelectedSignal.connect(std::move(cb)));
        return self();
    }
    TablePtr Table::onRowDoubleClicked(std::function<void(int)> cb)
    {
        if (cb) m_connections.push_back(onRowDoubleClickedSignal.connect(std::move(cb)));
        return self();
    }
    TablePtr Table::onSortRequested(std::function<void(int, bool)> cb)
    {
        if (cb) m_connections.push_back(onSortRequestedSignal.connect(std::move(cb)));
        return self();
    }
} // namespace mui
