// mui_icon_grid.cpp
#include "mui_icon_grid.hpp"
#include <imgui.h>
#include <algorithm>
#include <ctime>

// Original FontAwesome definitions from ImFileDialog
#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "\xef\x81\xbb"
#endif
#ifndef ICON_FA_FILE
#define ICON_FA_FILE "\xef\x85\x9b"
#endif

namespace mui {

void IconGrid::renderControl() {
    // Detail View (Table)
    if (zoomLevel == 1.0f) {
        if (ImGui::BeginTable("##IconGridTable", 3, ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY, ImVec2(0, 0))) {
            ImGui::TableSetupColumn("Name##filename", ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
            ImGui::TableSetupColumn("Date modified##filedate", ImGuiTableColumnFlags_WidthFixed, 150.0f, 1);
            ImGui::TableSetupColumn("Size##filesize", ImGuiTableColumnFlags_WidthFixed, 100.0f, 2);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            // Note: Sorting logic should ideally send an event back to the controller,
            // but for view-level rendering, we just read the items list sequentially.
            
            for (const auto& item : items) {
                ImGui::TableNextRow();
                bool isSelected = std::find(selectedIds.begin(), selectedIds.end(), item.id) != selectedIds.end();

                // Column 0: Name and Icon
                ImGui::TableSetColumnIndex(0);
                if (item.icon) {
                    ImGui::Image((ImTextureID)item.icon, ImVec2(16, 16));
                } else {
                    ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_Text], "%s", item.isDirectory ? ICON_FA_FOLDER : ICON_FA_FILE);
                }
                ImGui::SameLine();

                if (ImGui::Selectable(item.name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        if (onItemDoubleClicked) onItemDoubleClicked(item.id);
                    } else {
                        if (ImGui::GetIO().KeyCtrl) {
                            if (isSelected) {
                                selectedIds.erase(std::remove(selectedIds.begin(), selectedIds.end(), item.id), selectedIds.end());
                            } else {
                                selectedIds.push_back(item.id);
                            }
                        } else {
                            selectedIds.clear();
                            selectedIds.push_back(item.id);
                        }
                        if (onItemClicked) onItemClicked(item.id);
                    }
                }

                // Column 1: Date Modified
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(item.dateModified.c_str());
                // Column 2: Size
                ImGui::TableSetColumnIndex(2);
                if (!item.isDirectory) {
                    ImGui::Text("%.3f KiB", item.size / 1024.0f);
                } else {
                    ImGui::TextUnformatted("");
                }
            }
            ImGui::EndTable();
        }
    } 
    // Icon View (Grid Wrap)
    else {
        float windowVisibleX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        float iconBoxSize = 32.0f + 16.0f * zoomLevel;
        ImVec2 buttonSize(iconBoxSize, iconBoxSize + ImGui::GetFontSize() * 2);
        ImGuiStyle& style = ImGui::GetStyle();
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            ImGui::PushID(item.id.c_str());

            bool isSelected = std::find(selectedIds.begin(), selectedIds.end(), item.id) != selectedIds.end();
            
            // Draw custom InvisibleButton to catch clicks and draw selection box
            ImVec2 pos = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##icon_btn", buttonSize)) {
                if (ImGui::GetIO().KeyCtrl) {
                    if (isSelected) {
                        selectedIds.erase(std::remove(selectedIds.begin(), selectedIds.end(), item.id), selectedIds.end());
                    } else {
                        selectedIds.push_back(item.id);
                    }
                } else {
                    selectedIds.clear();
                    selectedIds.push_back(item.id);
                }
                if (onItemClicked) onItemClicked(item.id);
            }
            
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (onItemDoubleClicked) onItemDoubleClicked(item.id);
            }

            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            if (hovered || active || isSelected) {
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                    ImGui::ColorConvertFloat4ToU32(style.Colors[active ? ImGuiCol_HeaderActive : (isSelected ? ImGuiCol_Header : ImGuiCol_HeaderHovered)])
                );
            }

            // Draw Icon
            float iconDrawSize = buttonSize.y - ImGui::GetFontSize() * 2;
            float iconPosX = pos.x + (buttonSize.x - iconDrawSize) / 2.0f;
            
            if (item.icon) {
                // Determine if it needs preserving aspect ratio (thumbnails) or stretched (folder icons)
                ImGui::GetWindowDrawList()->AddImage((ImTextureID)item.icon, ImVec2(iconPosX, pos.y), ImVec2(iconPosX + iconDrawSize, pos.y + iconDrawSize));
            } else {
                const char* icon_text = item.isDirectory ? ICON_FA_FOLDER : ICON_FA_FILE;
                ImFont* font = ImGui::GetFont();
                ImVec2 textSize = font->CalcTextSizeA(iconDrawSize * 0.8f, FLT_MAX, 0.0f, icon_text);
                
                float textPosX = pos.x + (buttonSize.x - textSize.x) / 2.0f;
                float textPosY = pos.y + (iconDrawSize - textSize.y) / 2.0f;
                ImGui::GetWindowDrawList()->AddText(font, iconDrawSize * 0.8f, ImVec2(textPosX, textPosY), ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]), icon_text);
            }

            // Draw Label (truncated to fit)
            ImVec2 labelSize = ImGui::CalcTextSize(item.name.c_str(), nullptr, true, buttonSize.x);
            ImGui::GetWindowDrawList()->AddText(
                ImGui::GetFont(), ImGui::GetFontSize(), 
                ImVec2(pos.x + (buttonSize.x - labelSize.x) / 2.0f, pos.y + iconDrawSize), 
                ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]), 
                item.name.c_str(), nullptr, buttonSize.x
            );

            ImGui::PopID();

            // Calculate wrapping
            float lastButtonPos = ImGui::GetItemRectMax().x;
            float nextButtonPos = lastButtonPos + style.ItemSpacing.x + buttonSize.x;
            if (i + 1 < items.size() && nextButtonPos < windowVisibleX) {
                ImGui::SameLine();
            }
        }
    }
}

} // namespace mui

