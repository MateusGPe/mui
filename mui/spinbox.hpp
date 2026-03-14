#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <ui.h>

namespace mui
{
  class Spinbox;
  using SpinboxPtr = std::shared_ptr<Spinbox>;

  // --- Spinbox ---
  class Spinbox : public Control
  {
    uiSpinbox *spinbox;
    std::function<void()> onChangedCb;
    static void onChangedStub(uiSpinbox *s, void *data);

  public:
    Spinbox(int min, int max);

    template <typename... Args>
    static SpinboxPtr create(Args &&...args)
    {
      return std::make_shared<Spinbox>(std::forward<Args>(args)...);
    }

    int getValue() const;
    SpinboxPtr setValue(int val);
    SpinboxPtr onChanged(std::function<void()> cb);
  };

} // namespace mui
