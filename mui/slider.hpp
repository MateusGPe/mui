#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <ui.h>

namespace mui
{

  class Slider : public Control
  {
    uiSlider *slider;
    std::function<void()> onChangedCb;
    static void onChangedStub(uiSlider *s, void *data);

  public:
    Slider(int min, int max);

    template <typename... Args>
    static std::shared_ptr<Slider> create(Args &&...args)
    {
      return std::make_shared<Slider>(std::forward<Args>(args)...);
    }

    int getValue() const;
    std::shared_ptr<Slider> setValue(int val);
    std::shared_ptr<Slider> onChanged(std::function<void()> cb);
  };

} // namespace mui