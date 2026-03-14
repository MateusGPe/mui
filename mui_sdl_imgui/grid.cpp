#include "grid.hpp"
#include "app.hpp"
#include <imgui.h>
#include <string>
#include <algorithm>

namespace mui
{
    Grid::Grid() { App::assertMainThread(); }

    GridPtr Grid::append(ControlPtr child, int row, int col)
    {
        verifyState();
        cells[{row, col}] = child;
        maxRow = std::max(maxRow, row);
        maxCol = std::max(maxCol, col);
        return self();
    }

    void Grid::render()
    {
        if (!visible || cells.empty()) return;
        ImGui::PushID(this);
        
        if (ImGui::BeginTable("##grid", maxCol + 1)) {
            for (int r = 0; r <= maxRow; ++r) {
                ImGui::TableNextRow();
                for (int c = 0; c <= maxCol; ++c) {
                    ImGui::TableSetColumnIndex(c);
                    auto it = cells.find({r, c});
                    if (it != cells.end()) {
                        it->second->render();
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
    }
} // namespace mui
