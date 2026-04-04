// widgets/iconstack.cpp
#include "iconstack.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>

namespace mui
{
    IconStack::IconStack()
    {
        App::assertMainThread();
        inlineShadowEnabled = false;
    }

    void IconStack::renderControl()
    {
        if (!visible || icons.empty())
            return;

        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
        {
            ImGui::EndDisabled();
            return;
        }

        const ImGuiStyle &style = ImGui::GetStyle();
        ImFont *font = ImGui::GetFont();
        const float default_font_size = ImGui::GetFontSize();

        // --- Pass 1: Find natural max icon width ---
        float max_icon_width = 0.0f;
        for (const auto &icon : icons)
        {
            max_icon_width = std::max(max_icon_width, font->CalcTextSizeA(default_font_size, FLT_MAX, 0.0f, icon.icon.c_str()).x);
        }

        // Calculate single button natural boundaries and clamp to user-defined
        // Min/Max
        float base_btn_w = max_icon_width + style.FramePadding.x * 2.0f;
        float base_btn_h = ImGui::GetFrameHeight();

        base_btn_w = std::clamp(base_btn_w, m_minIconSize.x, m_maxIconSize.x);
        base_btn_h = std::clamp(base_btn_h, m_minIconSize.y, m_maxIconSize.y);

        // --- Pass 2: Calculate Natural Size of the ENTIRE Stack ---
        float natural_w = (base_btn_w * icons.size()) +
                          (style.ItemInnerSpacing.x * (icons.size() - 1));
        float natural_h = base_btn_h;

        // Apply our global fail-safe constraint resolver
        ImVec2 final_size = ApplySizeConstraints(ImVec2(natural_w, natural_h));

        // --- Pass 3: Distribute finalized constraints back to buttons ---
        // Evenly divide the constrained width among all buttons, subtracting the
        // spacing.
        float final_btn_w = std::max(
            1.0f, (final_size.x - (style.ItemInnerSpacing.x * (icons.size() - 1))) /
                      icons.size());
        float final_btn_h = std::max(1.0f, final_size.y);

        // Determine effective font size (Scale to Fit logic)
        float effective_font_size = default_font_size;
        if (m_scaleToFit)
        {
            float target_text_w =
                std::max(1.0f, final_btn_w - style.FramePadding.x * 2.0f);
            float target_text_h =
                std::max(1.0f, final_btn_h - style.FramePadding.y * 2.0f);

            // Scale uniformly to fit within both dimensions
            float scale_x = target_text_w / std::max(1.0f, max_icon_width);
            float scale_y = target_text_h / std::max(1.0f, default_font_size);

            effective_font_size = default_font_size * std::min(scale_x, scale_y);
        }

        // --- Pass 4: Render ---
        ImGui::BeginGroup();
        for (size_t i = 0; i < icons.size(); ++i)
        {
            ImGui::PushID((int)i);

            ImVec2 pos = window->DC.CursorPos;
            ImRect bb(pos, ImVec2(pos.x + final_btn_w, pos.y + final_btn_h));
            const ImGuiID btn_id = window->GetID("##btn");

            ImGui::ItemSize(bb);
            if (!ImGui::ItemAdd(bb, btn_id))
            {
                ImGui::PopID();
                continue;
            }

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, btn_id, &hovered, &held);

            if (pressed)
                onIconClickedSignal((int)i);

            // Render background frame
            ImGui::RenderFrame(bb.Min, bb.Max,
                               ImGui::GetColorU32((held && hovered)
                                                      ? ImGuiCol_ButtonActive
                                                  : hovered ? ImGuiCol_ButtonHovered
                                                            : ImGuiCol_Button),
                               true, style.FrameRounding);

            // Render scaled and centered text
            ImVec2 icon_size = font->CalcTextSizeA(effective_font_size, FLT_MAX, 0.0f,
                                                   icons[i].icon.c_str());
            ImVec2 icon_pos(bb.Min.x + (final_btn_w - icon_size.x) * 0.5f,
                            bb.Min.y + (final_btn_h - icon_size.y) * 0.5f);

            ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
            if (icons[i].text_hover)
            {
                text_col = ImGui::GetColorU32(held      ? ImGuiCol_TextDisabled
                                              : hovered ? ImGuiCol_Text
                                                        : ImGuiCol_TextDisabled);
            }

            // Bypass standard ImGui::RenderText to inject our custom font scale
            window->DrawList->AddText(font, effective_font_size, icon_pos, text_col,
                                      icons[i].icon.c_str());

            if (!icons[i].tooltip.empty() && ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", icons[i].tooltip.c_str());
            }

            ImGui::PopID();

            if (i < icons.size() - 1)
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
        }
        ImGui::EndGroup();

        renderTooltip(); // Ensure parent Control tooltip capability is fired off if
                         // set

        ImGui::EndDisabled();
    }

    IconStackPtr IconStack::add(const std::string &icon,
                                std::function<void()> onClick,
                                const std::string &tip, bool textHover)
    {
        int index = (int)icons.size();
        icons.push_back({icon, tip, textHover, -1.0f});

        if (onClick)
        {
            addConnection(onIconClickedSignal.connect(
                [index, cb = std::move(onClick)](int clickedIndex)
                {
                    if (clickedIndex == index)
                        cb();
                }));
        }

        return self();
    }
} // namespace mui