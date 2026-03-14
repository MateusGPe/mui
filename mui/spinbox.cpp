#include "spinbox.hpp"
#include "app.hpp"

namespace mui
{

  MUI_IMPL_STATELESS_CB(Spinbox, uiSpinbox, onChangedStub, onChangedCb)

  Spinbox::Spinbox(int min, int max)
  {
    App::assertMainThread();
    spinbox = uiNewSpinbox(min, max);
    handle = uiControl(spinbox);
    uiSpinboxOnChanged(spinbox, onChangedStub, this);
  }

  int Spinbox::getValue() const
  {
    verifyState();
    return uiSpinboxValue(spinbox);
  }

  SpinboxPtr Spinbox::setValue(int val)
  {
    verifyState();
    uiSpinboxSetValue(spinbox, val);
    return self();
  }

  SpinboxPtr Spinbox::onChanged(std::function<void()> cb)
  {
    verifyState();
    onChangedCb = std::move(cb);
    return self();
  }

} // namespace mui
