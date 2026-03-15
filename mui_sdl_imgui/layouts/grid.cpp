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

    void Grid::renderControl()
    {
        if (!visible || cells.empty()) return;
        ImGui::PushID(this);

        if (ImGui::BeginTable("##grid", maxCol + 1, ImGuiTableFlags_NoClip)) {
            for (int r = 0; r <= maxRow; ++r) {
                ImGui::TableNextRow();
                for (int c = 0; c <= maxCol; ) {
                    auto it = cells.find({r, c});
                    if (it != cells.end()) {
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
                            cell.control->render();
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
