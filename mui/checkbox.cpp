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
    const char *text = uiCheckboxText(checkbox);
    return text ? std::string(text) : std::string();
  }

  void Checkbox::setText(const std::string &text)
  {
    verifyState();
    uiCheckboxSetText(checkbox, text.c_str());
  }

  std::shared_ptr<Checkbox> Checkbox::setChecked(bool checked)
  {
    verifyState();
    uiCheckboxSetChecked(checkbox, checked ? 1 : 0);
    return std::static_pointer_cast<Checkbox>(shared_from_this());
  }

  std::shared_ptr<Checkbox> Checkbox::onToggled(std::function<void()> cb)
  {
    verifyState();
    onToggledCb = std::move(cb);
    return std::static_pointer_cast<Checkbox>(shared_from_this());
  }

} // namespace mui