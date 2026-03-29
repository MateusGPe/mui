#include "progressbar.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <algorithm>
#include <imgui.h>

namespace mui
{
  ProgressBar::ProgressBar() : value(0.0f) { App::assertMainThread(); }

  void ProgressBar::renderControl()
  {
    if (!visible)
      return;
    ScopedControlID id(this);

    float w = width;
    if (spanAvailWidth)
      w = -FLT_MIN;
    else if (useContainerWidth)
      w = ImGui::CalcItemWidth();
    ImVec2 size(w, 0);
    const char *overlay = overlayText.empty() ? NULL : overlayText.c_str();

    ImGui::ProgressBar(value, size, overlay);

    renderTooltip();
  }

  ProgressBarPtr ProgressBar::setValue(float v)
  {
    value = std::clamp(v, 0.0f, 1.0f);
    return self();
  }
  ProgressBarPtr ProgressBar::setOverlayText(const std::string &text)
  {
    overlayText = text;
    return self();
  }
  ProgressBarPtr ProgressBar::setSpanAvailWidth(bool span)
  {
    spanAvailWidth = span;
    return self();
  }

  ProgressBarPtr ProgressBar::setUseContainerWidth(bool use)
  {
    useContainerWidth = use;
    return self();
  }
} // namespace mui
