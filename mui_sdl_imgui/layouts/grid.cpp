#include "grid.hpp"
#include "app.hpp"
#include <imgui.h>
#include <string>
#include <algorithm>

namespace mui
{
    Grid::Grid() { App::assertMainThread(); }

    GridPtr Grid::append(ControlPtr child, int row, int col, int colSpan)
    {
        verifyState();
        m_cells[{row, col}] = {child, colSpan};
        m_maxRow = std::max(m_maxRow, row);
        m_maxCol = std::max(m_maxCol, col + colSpan - 1);
        return self();
    }

    GridPtr Grid::setColumnWeight(int col, float weight)
    {
        verifyState();
        m_columnWeights[col] = weight;
        return self();
    }

    void Grid::renderControl()
    {
        if (!visible || m_cells.empty()) return;
        ImGui::PushID(this);

        ImGuiTableFlags flags = ImGuiTableFlags_NoClip;
        if (!m_columnWeights.empty()) {
            flags |= ImGuiTableFlags_SizingStretchProp;
        }

        if (ImGui::BeginTable("##grid", m_maxCol + 1, flags)) {
            if (!m_columnWeights.empty()) {
                for (int c = 0; c <= m_maxCol; ++c) {
                    auto it = m_columnWeights.find(c);
                    float weight = (it != m_columnWeights.end()) ? it->second : 1.0f;
                    ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, weight);
                }
            }

            for (int r = 0; r <= m_maxRow; ++r) {
                ImGui::TableNextRow();
                for (int c = 0; c <= m_maxCol; ) {
                    auto it = m_cells.find({r, c});
                    if (it != m_cells.end()) {
                        const auto &cell = it->second;
                        ImGui::TableSetColumnIndex(c);
                        
                        if (cell.colSpan > 1) {
                            ImGui::TableSetColumnIndex(c + cell.colSpan - 1);
                            float endX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
                            ImGui::TableSetColumnIndex(c);
                            float startX = ImGui::GetCursorPosX();
                            ImGui::PushItemWidth(endX - startX);
                            cell.control->render();
                            ImGui::PopItemWidth();
                            c += cell.colSpan;
                        } else {
                            ImGui::PushItemWidth(-FLT_MIN);
                            cell.control->render();
                            ImGui::PopItemWidth();
                            c++;
                        }
                    } else {
                        c++;
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
    }
} // namespace mui
