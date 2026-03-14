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

  SliderPtr Slider::setValue(int val)
  {
    verifyState();
    uiSliderSetValue(slider, val);
    return self();
  }

  SliderPtr Slider::onChanged(std::function<void()> cb)
  {
    verifyState();
    onChangedCb = std::move(cb);
    return self();
  }

} // namespace mui