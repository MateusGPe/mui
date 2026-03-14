#include "button.hpp"
#include "app.hpp"

namespace mui
{

  void Button::onClickedStub(uiButton *b, void *data)
  {
    auto self = static_cast<Button *>(data);
    if (self->onClickCb)
      self->onClickCb();
  }

  Button::Button(const std::string &text)
  {
    App::assertMainThread();
    btn = uiNewButton(text.c_str());
    handle = uiControl(btn);
    uiButtonOnClicked(btn, onClickedStub, this);
  }

  std::string Button::getText() const
  {
    verifyState();
    const char *text = uiButtonText(btn);
    return text ? std::string(text) : std::string();
  }

  void Button::setText(const std::string &text)
  {
    verifyState();
    uiButtonSetText(btn, text.c_str());
  }

  std::shared_ptr<Button> Button::onClick(std::function<void()> cb)
  {
    verifyState();
    onClickCb = std::move(cb);
    return std::static_pointer_cast<Button>(shared_from_this());
  }

} // namespace mui