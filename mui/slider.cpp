#include "slider.hpp"
#include "app.hpp"

namespace mui
{

  MUI_IMPL_STATELESS_CB(Slider, uiSlider, onChangedStub, onChangedCb)

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