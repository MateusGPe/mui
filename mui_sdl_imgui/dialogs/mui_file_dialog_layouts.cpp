// mui_file_dialog_layouts.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "mui_file_dialog_layouts.hpp"
#include <imgui.h>
#include <imgui_internal.h>

#define GUI_ELEMENT_SIZE std::max(ImGui::GetFontSize() + 10.f, 24.f)

namespace mui {

void FileDialogTopBar::renderControl() {
    ImGui::PushStyleColor(ImGuiCol_Button, 0);

    // Navigation Buttons
    if (ImGui::ArrowButtonEx("##back", ImGuiDir_Left, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE))) {
        if (onBackClicked) onBackClicked();
    }
    ImGui::SameLine();

    if (ImGui::ArrowButtonEx("##forward", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE))) {
        if (onForwardClicked) onForwardClicked();
    }
    ImGui::SameLine();

    if (ImGui::ArrowButtonEx("##up", ImGuiDir_Up, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE))) {
        if (onUpClicked) onUpClicked();
    }
    ImGui::SameLine();

    ImGui::PopStyleColor();

    // Breadcrumb goes here in the master controller (injected between these elements)
    // Note: The controller typically renders the BreadcrumbBar directly after this call
    // or you can inject a pointer to it inside this layout.

    // Search Box
    char searchBuf[128];
    strncpy(searchBuf, searchInput.c_str(), sizeof(searchBuf));
    
    ImGui::SameLine();
    if (ImGui::InputTextEx("##searchTB", "Search", searchBuf, 128, ImVec2(-FLT_MIN, GUI_ELEMENT_SIZE), 0)) {
        searchInput = searchBuf;
        if (onSearchChanged) onSearchChanged(searchInput);
    }
}

void FileDialogBottomBar::renderControl() {
    ImGui::Text("File name:");
    ImGui::SameLine();
    
    char fileBuf[1024];
    strncpy(fileBuf, filenameInput.c_str(), sizeof(fileBuf));

    // Filename input spans differently based on mode
    float inputWidth = isDirectoryMode ? -FLT_MIN : -250.0f;
    
    if (ImGui::InputTextEx("##file_input", "", fileBuf, 1024, ImVec2(inputWidth, 0), ImGuiInputTextFlags_EnterReturnsTrue)) {
        filenameInput = fileBuf;
        if (onFilenameChanged) onFilenameChanged(filenameInput);
        if (onOkClicked) onOkClicked();
    }
    
    if (!isDirectoryMode && !filters.empty()) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);

        // Build combo box string separated by \0
        std::string comboStr = "";
        for (const auto& f : filters) {
            comboStr += f.label + '\0';
        }
        comboStr += '\0';

        if (ImGui::Combo("##ext_combo", &selectedFilterIndex, comboStr.c_str())) {
            if (onFilterChanged) onFilterChanged(selectedFilterIndex);
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 150);
    if (ImGui::Button("Open", ImVec2(60, 0))) {
        filenameInput = fileBuf; // Ensure we grab latest text before OK
        if (onOkClicked) onOkClicked();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(60, 0))) {
        if (onCancelClicked) onCancelClicked();
    }
}

} // namespace mui
