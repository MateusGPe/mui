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
        cells[{row, col}] = {child, colSpan};
        maxRow = std::max(maxRow, row);
        maxCol = std::max(maxCol, col + colSpan - 1);
        return self();
    }

    void Grid::render()
    {
        if (!visible || cells.empty()) return;
        ImGui::PushID(this);

        // ImGuiTableFlags_NoClip is required so the spanned widget isn't visually truncated by the first column's boundaries.
        if (ImGui::BeginTable("##grid", maxCol + 1, ImGuiTableFlags_NoClip)) {
            for (int r = 0; r <= maxRow; ++r) {
                ImGui::TableNextRow();
                for (int c = 0; c <= maxCol; ++c) {
                    if (auto it = cells.find({r, c}); it != cells.end()) {
                        ImGui::TableSetColumnIndex(c);
                        const auto &cell = it->second;

                        // Workaround for missing native colspan
                        if (cell.colSpan > 1) {
                            // 1. Record starting X position
                            float startX = ImGui::GetCursorPosX();
                            
                            // 2. Jump to the final spanned column to measure the end X position
                            ImGui::TableSetColumnIndex(c + cell.colSpan - 1);
                            float endX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
                            
                            // 3. Return to the original column and force the computed width
                            ImGui::TableSetColumnIndex(c);
                            ImGui::PushItemWidth(endX - startX);
                            cell.control->render();
                            ImGui::PopItemWidth();
                        } else {
                        cell.control->render();
                        }

                        c += cell.colSpan - 1; // Skip the columns that are now spanned
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
    }
} // namespace mui
