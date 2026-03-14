#include "label.hpp"
#include "app.hpp"

namespace mui
{

  Label::Label(const std::string &text)
  {
    App::assertMainThread();
    label = uiNewLabel(text.c_str());
    handle = uiControl(label);
  }

  LabelPtr Label::setText(const std::string &text)
  {
    verifyState();
    uiLabelSetText(label, text.c_str());
    return std::static_pointer_cast<Label>(shared_from_this());
  }

  std::string Label::getText() const
  {
    verifyState();
    return UiText(uiLabelText(label));
  }

} // namespace mui