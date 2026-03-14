#include "card.hpp"
#include "app.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace mui
{
    Card::Card() {
        App::assertMainThread();
    }

    void Card::render()
    {
        if (!visible) return;

        ImGui::PushID(this);

        // Render child into a group to measure its size
        ImGui::BeginGroup();
        if (child) {
            child->render();
        }
        ImGui::EndGroup();

        // Get the bounding box of the child group
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();

        // Add padding
        p0.x -= padding; p0.y -= padding;
        p1.x += padding; p1.y += padding;

        // 1. Draw Shadow
        if (shadowAlpha > 0) {
            // AddShadowRect is not a standard ImGui function.
            // We simulate a simple shadow with AddRectFilled. This does not support blur.
            ImVec2 shadow_p0 = ImVec2(p0.x + shadowOffset, p0.y + shadowOffset);
            ImVec2 shadow_p1 = ImVec2(p1.x + shadowOffset, p1.y + shadowOffset);
            draw_list->AddRectFilled(shadow_p0, shadow_p1, IM_COL32(0, 0, 0, shadowAlpha), 4.0f);
        }

        // 2. Draw Background
        draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_ChildBg), 4.0f);

        // 3. Draw Border
        draw_list->AddRect(p0, p1, ImGui::GetColorU32(ImGuiCol_Border), 4.0f);

        ImGui::PopID();
    }

    CardPtr Card::setChild(ControlPtr c) {
        verifyState();
        child = c;
        return self();
    }

    CardPtr Card::setPadding(float p) {
        verifyState();
        padding = p;
        return self();
    }

    CardPtr Card::setShadow(float blur, float offset, int alpha) {
        verifyState();
        shadowBlur = blur;
        shadowOffset = offset;
        shadowAlpha = alpha;
        return self();
    }
} // namespace mui