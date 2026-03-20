// widgets/toggleswitch.cpp
#include "toggleswitch.hpp"
#include "app.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace mui
{
    ToggleSwitch::ToggleSwitch(const std::string &label) : label(label), checked(false) { App::assertMainThread(); }

    void ToggleSwitch::renderControl()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::EndDisabled();
            ImGui::PopID();
            return;
        }

        const ImGuiStyle &style = ImGui::GetStyle();
        const ImGuiID id = window->GetID(label.c_str());

        // Calculate dimensions
        const float switch_height = ImGui::GetFrameHeight() * scale;
        const float switch_width = switch_height * 1.8f;
        const ImVec2 label_size = ImGui::CalcTextSize(label.c_str(), NULL, true);

        // Calculate total bounding box for the whole widget (switch + label)
        const float frame_height = std::max(switch_height, label_size.y);
        const ImRect total_bb(window->DC.CursorPos,
                              ImVec2(window->DC.CursorPos.x + switch_width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f),
                                     window->DC.CursorPos.y + frame_height));

        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, id))
        {
            ImGui::EndDisabled();
            ImGui::PopID();
            return;
        }

        // Use the total bounding box for interaction
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        if (pressed)
        {
            checked = !checked;
            if (onToggledCb)
                onToggledCb(checked);
        }

        // --- Render ---
        const float switch_y_offset = (frame_height - switch_height) / 2.0f;
        const ImRect switch_bb(ImVec2(total_bb.Min.x, total_bb.Min.y + switch_y_offset), ImVec2(total_bb.Min.x + switch_width, total_bb.Min.y + switch_y_offset + switch_height));
        const float radius = switch_height * 0.50f;

        ImU32 col_bg = ImGui::GetColorU32(checked ? ((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button) : ((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
        window->DrawList->AddRectFilled(switch_bb.Min, switch_bb.Max, col_bg, radius);
        float t = checked ? 1.0f : 0.0f;
        window->DrawList->AddCircleFilled(ImVec2(switch_bb.Min.x + radius + t * (switch_width - radius * 2.0f), switch_bb.Min.y + radius), radius - 2.0f, ImGui::GetColorU32(ImGuiCol_Text));

        if (label_size.x > 0.0f)
        {
            const float label_y_offset = (frame_height - label_size.y) / 2.0f;
            ImGui::RenderText(ImVec2(switch_bb.Max.x + style.ItemInnerSpacing.x, total_bb.Min.y + label_y_offset), label.c_str());
        }

        renderTooltip();
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    ToggleSwitchPtr ToggleSwitch::setChecked(bool c) { checked = c; return self(); }
    ToggleSwitchPtr ToggleSwitch::onToggled(std::function<void(bool)> cb) { onToggledCb = std::move(cb); return self(); }

    ToggleSwitchPtr ToggleSwitch::setScale(float s)
    {
        scale = s;
        return self();
    }
}
