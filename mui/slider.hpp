#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <ui.h>

namespace mui
{
  class Slider;
  using SliderPtr = std::shared_ptr<Slider>;

  class Slider : public Control
  {
    uiSlider *slider;
    std::function<void()> onChangedCb;
    static void onChangedStub(uiSlider *s, void *data);

  public:
    Slider(int min, int max);

    template <typename... Args>
    static SliderPtr create(Args &&...args)
    {
      return std::make_shared<Slider>(std::forward<Args>(args)...);
    }

    int getValue() const;
    SliderPtr setValue(int val);
    SliderPtr onChanged(std::function<void()> cb);
  };

} // namespace mui