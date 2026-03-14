#include "spinbox.hpp"
#include "app.hpp"

namespace mui
{

  void Spinbox::onChangedStub(uiSpinbox *s, void *data)
  {
    auto self = static_cast<Spinbox *>(data);
    if (self->onChangedCb)
      self->onChangedCb();
  }

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
