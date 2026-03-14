#include "slider.hpp"
#include "app.hpp"

namespace mui
{

  void Slider::onChangedStub(uiSlider *s, void *data)
  {
    auto self = static_cast<Slider *>(data);
    if (self->onChangedCb)
      self->onChangedCb();
  }

  Slider::Slider(int min, int max)
  {
    App::assertMainThread();
    slider = uiNewSlider(min, max);
    handle = uiControl(slider);
    uiSliderOnChanged(slider, onChangedStub, this);
  }

  int Slider::getValue() const
  {
    verifyState();
    return uiSliderValue(slider);
  }

  std::shared_ptr<Slider> Slider::setValue(int val)
  {
    verifyState();
    uiSliderSetValue(slider, val);
    return std::static_pointer_cast<Slider>(shared_from_this());
  }

  std::shared_ptr<Slider> Slider::onChanged(std::function<void()> cb)
  {
    verifyState();
    onChangedCb = std::move(cb);
    return std::static_pointer_cast<Slider>(shared_from_this());
  }

} // namespace mui