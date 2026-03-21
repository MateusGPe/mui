// widgets/rangeslider.cpp
#include "rangeslider.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"
#include <imgui_internal.h>

namespace mui
{
    RangeSlider::RangeSlider(float min, float max)
        : minBound(min), maxBound(max), currentMin(min), currentMax(max) { App::assertMainThread(); }

    void RangeSlider::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);

        auto render_slider = [&]
        {
            ImGuiWindow *window = ImGui::GetCurrentWindow();
            if (window->SkipItems)
                return;

            ImGuiContext &g = *GImGui;
            const ImGuiStyle &style = g.Style;
            const ImGuiID slider_id = window->GetID("##range");

            float w = ImGui::CalcItemWidth();
            const ImVec2 label_size = ImGui::CalcTextSize(text.c_str(), NULL, true);
            const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2.0f));
            const ImRect total_bb(frame_bb.Min, ImVec2(frame_bb.Max.x + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), frame_bb.Max.y));

            ImGui::ItemSize(total_bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(total_bb, slider_id, &frame_bb, 0))
                return;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(frame_bb, slider_id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

            // Render background track
            ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

            const float grab_padding = 2.0f;
            const float grab_sz = ImMin(style.GrabMinSize, frame_bb.GetHeight() - grab_padding * 2.0f);
            const float slider_usable_sz = frame_bb.GetWidth() - grab_sz;
            const float slider_usable_pos_min = frame_bb.Min.x + grab_sz * 0.5f;

            auto calc_t = [&](float v)
            { return ImClamp((v - minBound) / (maxBound - minBound), 0.0f, 1.0f); };
            float tMin = calc_t(currentMin);
            float tMax = calc_t(currentMax);

            float grabMinX = slider_usable_pos_min + tMin * slider_usable_sz;
            float grabMaxX = slider_usable_pos_min + tMax * slider_usable_sz;

            bool value_changed = false;
            ImGuiStorage *storage = ImGui::GetStateStorage();
            int active_handle = storage->GetInt(slider_id, 0); // 1 for min, 2 for max

            if (held)
            {
                float mouse_x = g.IO.MousePos.x;
                float new_t = ImClamp((mouse_x - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f);
                float new_val = minBound + new_t * (maxBound - minBound);

                if (g.ActiveIdIsJustActivated)
                {
                    float d1 = ImFabs(mouse_x - grabMinX);
                    float d2 = ImFabs(mouse_x - grabMaxX);
                    active_handle = (d1 <= d2) ? 1 : 2;
                    storage->SetInt(slider_id, active_handle);
                }

                if (active_handle == 1)
                {
                    if (currentMin != new_val)
                    {
                        currentMin = new_val;
                        if (currentMin > currentMax)
                            currentMax = currentMin;
                        value_changed = true;
                    }
                }
                else if (active_handle == 2)
                {
                    if (currentMax != new_val)
                    {
                        currentMax = new_val;
                        if (currentMax < currentMin)
                            currentMin = currentMax;
                        value_changed = true;
                    }
                }
            }
            else
            {
                storage->SetInt(slider_id, 0);
            }

            // Recalculate grab positions after change
            tMin = calc_t(currentMin);
            tMax = calc_t(currentMax);
            grabMinX = slider_usable_pos_min + tMin * slider_usable_sz;
            grabMaxX = slider_usable_pos_min + tMax * slider_usable_sz;

            // Highlight track between handles
            ImRect highlight_bb(ImVec2(grabMinX, frame_bb.Min.y + grab_padding), ImVec2(grabMaxX, frame_bb.Max.y - grab_padding));
            window->DrawList->AddRectFilled(highlight_bb.Min, highlight_bb.Max, ImGui::GetColorU32(ImGuiCol_Header), style.FrameRounding);

            // Draw grabs
            auto draw_grab = [&](float x, bool is_active)
            {
                ImRect grab_bb(ImVec2(x - grab_sz * 0.5f, frame_bb.Min.y + grab_padding), ImVec2(x + grab_sz * 0.5f, frame_bb.Max.y - grab_padding));
                ImU32 col = ImGui::GetColorU32(is_active ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);
                window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, col, style.GrabRounding);
            };

            bool is_min_active = (held && active_handle == 1);
            bool is_max_active = (held && active_handle == 2);
            draw_grab(grabMinX, is_min_active);
            draw_grab(grabMaxX, is_max_active);

            // Tooltips based on hover or drag
            if (held || hovered)
            {
                char val_buf[32];
                if (active_handle == 1 || (hovered && ImFabs(g.IO.MousePos.x - grabMinX) <= ImFabs(g.IO.MousePos.x - grabMaxX) && active_handle == 0))
                {
                    snprintf(val_buf, sizeof(val_buf), format.c_str(), currentMin);
                    ImGui::SetTooltip("%s", val_buf);
                }
                else if (active_handle == 2 || (hovered && active_handle == 0))
                {
                    snprintf(val_buf, sizeof(val_buf), format.c_str(), currentMax);
                    ImGui::SetTooltip("%s", val_buf);
                }
            }

            // Draw current values centered on the slider
            char bufMin[32], bufMax[32], bufFull[64];
            snprintf(bufMin, sizeof(bufMin), format.c_str(), currentMin);
            snprintf(bufMax, sizeof(bufMax), format.c_str(), currentMax);
            snprintf(bufFull, sizeof(bufFull), "%s - %s", bufMin, bufMax);
            ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, bufFull, NULL, NULL, ImVec2(0.5f, 0.5f));

            if (label_size.x > 0.0f)
            {
                ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), text.c_str());
            }

            if (value_changed && onChangedCb)
                onChangedCb(currentMin, currentMax);
        };

        {
            ScopedItemWidth width;
            if (spanAvailWidth)
            {
                width.push(-FLT_MIN);
            }
            else if (useContainerWidth)
            {
                // Do nothing, will inherit width from container
            }
            render_slider();
        }

        renderTooltip();
    }

    RangeSliderPtr RangeSlider::setText(const std::string &t)
    {
        text = t;
        return self();
    }

    RangeSliderPtr RangeSlider::setFormat(const std::string &f)
    {
        format = f;
        return self();
    }

    RangeSliderPtr RangeSlider::setRange(float vMin, float vMax)
    {
        currentMin = vMin;
        currentMax = vMax;
        return self();
    }
    RangeSliderPtr RangeSlider::onChanged(std::function<void(float, float)> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }

    RangeSliderPtr RangeSlider::setUseContainerWidth(bool use)
    {
        useContainerWidth = use;
        return self();
    }
}
