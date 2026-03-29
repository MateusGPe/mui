// widgets/checkbox.cpp
#include "checkbox.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>

namespace mui
{
  Checkbox::Checkbox(const std::string &text) : text(text), checked(false)
  {
    App::assertMainThread();
  }

  void Checkbox::renderControl()
  {
    if (!visible)
      return;
    ScopedControlID sid(this);
    ImGui::BeginDisabled(!enabled);

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
      ImGui::EndDisabled();
      return;
    }

    const ImGuiStyle &style = ImGui::GetStyle();
    const ImGuiID m_id = window->GetID(text.c_str());

    const float check_box_size = ImGui::GetFrameHeight() * scale;
    const ImVec2 label_size = ImGui::CalcTextSize(text.c_str(), NULL, true);

    float w = width;
    if (spanAvailWidth)
      w = ImGui::GetContentRegionAvail().x;
    else if (useContainerWidth)
      w = ImGui::CalcItemWidth();
    if (w <= 0)
      w = check_box_size +
          (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f);

    float h = height > 0 ? height
                         : std::max(check_box_size, label_size.y) +
                               style.FramePadding.y * 2;

    w = std::clamp(w, minSize.x, maxSize.x);
    h = std::clamp(h, minSize.y, maxSize.y);

    const ImRect total_bb(
        window->DC.CursorPos,
        ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.y + h));

    ImGui::ItemSize(total_bb, 0.0f);
    if (!ImGui::ItemAdd(total_bb, m_id))
    {
      ImGui::EndDisabled();
      return;
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, m_id, &hovered, &held);
    if (pressed)
    {
      checked = !checked;
      onToggledSignal(checked);
    }

    const float check_y_offset = (h - check_box_size) / 2.0f;
    const ImRect check_bb(
        ImVec2(total_bb.Min.x, total_bb.Min.y + check_y_offset),
        ImVec2(total_bb.Min.x + check_box_size,
               total_bb.Min.y + check_y_offset + check_box_size));

    ImGui::RenderFrame(check_bb.Min, check_bb.Max,
                       ImGui::GetColorU32((held && hovered)
                                              ? ImGuiCol_FrameBgActive
                                          : hovered ? ImGuiCol_FrameBgHovered
                                                    : ImGuiCol_FrameBg),
                       true, style.FrameRounding);
    if (checked)
    {
      const float pad = std::max(1.0f, (float)(int)(check_box_size / 6.0f));
      ImGui::RenderCheckMark(
          window->DrawList, ImVec2(check_bb.Min.x + pad, check_bb.Min.y + pad),
          ImGui::GetColorU32(ImGuiCol_CheckMark), check_box_size - pad * 2.0f);
    }

    if (label_size.x > 0.0f)
    {
      const float label_y_offset = (h - label_size.y) / 2.0f;
      ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x,
                               total_bb.Min.y + label_y_offset),
                        text.c_str());
    }

    ImGui::EndDisabled();
  }

  bool Checkbox::isChecked() const { return checked; }
  std::string Checkbox::getText() const { return text; }
  CheckboxPtr Checkbox::setText(const std::string &t)
  {
    text = t;
    return self();
  }

  CheckboxPtr Checkbox::setChecked(bool c)
  {
    checked = c;
    return self();
  }

  CheckboxPtr Checkbox::setScale(float s)
  {
    scale = s;
    return self();
  }

  CheckboxPtr Checkbox::bind(std::shared_ptr<Observable<bool>> observable)
  {
    setChecked(observable->get());

    m_connections.push_back(
        observable->onValueChanged.connect([this](const bool &val)
                                           { mui::App::queueMain([this, val]()
                                                                 { this->setChecked(val); }); }));

    m_connections.push_back(onToggledSignal.connect(
        [observable](bool val)
        { observable->set(val); }));

    return self();
  }

  CheckboxPtr Checkbox::onToggled(std::function<void(bool)> cb)
  {
    if (cb)
      m_connections.push_back(onToggledSignal.connect(std::move(cb)));
    return self();
  }
} // namespace mui
