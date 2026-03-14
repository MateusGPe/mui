#include "checkbox.hpp"
#include "app.hpp"

namespace mui
{

  void Checkbox::onToggledStub(uiCheckbox *c, void *data)
  {
    auto self = static_cast<Checkbox *>(data);
    if (self->onToggledCb)
      self->onToggledCb();
  }

  Checkbox::Checkbox(const std::string &text)
  {
    App::assertMainThread();
    checkbox = uiNewCheckbox(text.c_str());
    handle = uiControl(checkbox);
    uiCheckboxOnToggled(checkbox, onToggledStub, this);
  }

  bool Checkbox::isChecked() const
  {
    verifyState();
    return uiCheckboxChecked(checkbox) != 0;
  }

  std::string Checkbox::getText() const
  {
    verifyState();
    return UiText(uiCheckboxText(checkbox));
  }

  void Checkbox::setText(const std::string &text)
  {
    verifyState();
    uiCheckboxSetText(checkbox, text.c_str());
  }

  CheckboxPtr Checkbox::setChecked(bool checked)
  {
    verifyState();
    uiCheckboxSetChecked(checkbox, checked ? 1 : 0);
    return self();
  }

  CheckboxPtr Checkbox::onToggled(std::function<void()> cb)
  {
    verifyState();
    onToggledCb = std::move(cb);
    return self();
  }

} // namespace mui