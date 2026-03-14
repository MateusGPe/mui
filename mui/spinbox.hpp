#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <ui.h>

namespace mui
{

  class Spinbox : public Control
  {
    uiSpinbox *spinbox;
    std::function<void()> onChangedCb;
    static void onChangedStub(uiSpinbox *s, void *data);

  public:
    Spinbox(int min, int max);

    template <typename... Args>
    static std::shared_ptr<Spinbox> create(Args &&...args)
    {
      return std::make_shared<Spinbox>(std::forward<Args>(args)...);
    }

    int getValue() const;
    std::shared_ptr<Spinbox> setValue(int val);
    std::shared_ptr<Spinbox> onChanged(std::function<void()> cb);
  };

} // namespace mui
