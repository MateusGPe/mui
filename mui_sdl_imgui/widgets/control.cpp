#include "control.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include <imgui_internal.h> // Required for AddShadowRect if using older ImGui versions

namespace mui
{
    Control::~Control() {}

    void Control::onHandleDestroyed() { ownsHandle = false; }
    void Control::verifyState() const { App::assertMainThread(); }

    void Control::show() { visible = true; }
    void Control::hide() { visible = false; }

    void Control::setEnabled(bool enabled) { this->enabled = enabled; }
    bool Control::isEnabled() const { return enabled; }

    void Control::releaseOwnership() { ownsHandle = false; }
    void Control::acquireOwnership() { ownsHandle = true; }

    ControlPtr Control::setTooltip(const std::string &t)
    {
        tooltip = t;
        return shared_from_this();
    }

    void Control::renderTooltip() {
        if (!tooltip.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", tooltip.c_str());
        }
    }

    ControlPtr Control::setSpanAvailWidth(bool span)
    {
        spanAvailWidth = span;
        return shared_from_this();
    }

    ControlPtr Control::setShadow(bool enable, ImVec2 offset, float blur, ImVec4 col, float rounding)
    {
        hasShadow = enable;
        shadowOffset = offset;
        shadowBlur = blur;
        shadowColor = col;
        shadowRounding = rounding;
        return shared_from_this();
    }

    void Control::render() {
        if (!visible) return;

        if (!hasShadow) {
            renderControl();
            return;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // 1. Split channels
        draw_list->ChannelsSplit(2);
        draw_list->ChannelsSetCurrent(1); // Foreground

        // 2. Render widget to get dimensions
        ImGui::BeginGroup();
        renderControl();
        ImGui::EndGroup();

        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();

        // 3. Switch to background to draw shadow
        draw_list->ChannelsSetCurrent(0);

        float rounding = shadowRounding < 0.0f ? ImGui::GetStyle().FrameRounding : shadowRounding;

        if (shadowBlur <= 0.0f) {
            // Hard/Flat Retro Shadow
            draw_list->AddRectFilled(
                ImVec2(p_min.x + shadowOffset.x, p_min.y + shadowOffset.y),
                ImVec2(p_max.x + shadowOffset.x, p_max.y + shadowOffset.y),
                ImGui::GetColorU32(shadowColor),
                rounding
            );
        } else {
            // Simulated Soft Shadow (Hack)
            // Warning: High iterations will spike draw calls. Keep between 3 and 5.
            const int iterations = 4; 
            for (int i = 1; i <= iterations; ++i) {
                float step = (shadowBlur / iterations) * i;
                // Fade alpha outwards
                float alpha = shadowColor.w * (1.0f - ((float)i / iterations));
                ImU32 col = ImGui::GetColorU32(ImVec4(shadowColor.x, shadowColor.y, shadowColor.z, alpha));
                
                draw_list->AddRectFilled(
                    ImVec2(p_min.x + shadowOffset.x - step, p_min.y + shadowOffset.y - step),
                    ImVec2(p_max.x + shadowOffset.x + step, p_max.y + shadowOffset.y + step),
                    col, 
                    rounding + step
                );
            }
        }

        // 4. Merge
        draw_list->ChannelsMerge();
    }
} // namespace mui