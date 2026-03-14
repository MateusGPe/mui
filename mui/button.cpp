#include "button.hpp"
#include "app.hpp"

namespace mui
{
  MUI_IMPL_STATELESS_CB(Button, uiButton, onClickedStub, onClickCb)

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
    return UiText(uiButtonText(btn));
  }

  void Button::setText(const std::string &text)
  {
    verifyState();
    uiButtonSetText(btn, text.c_str());
  }

  ButtonPtr Button::onClick(std::function<void()> cb)
  {
    verifyState();
    onClickCb = std::move(cb);
    return self();
  }

} // namespace mui