#pragma once
#include "control.hpp"
#include <memory>
#include <ui.h>

namespace mui
{

  class ProgressBar : public Control
  {
    uiProgressBar *progress;

  public:
    ProgressBar();

    template <typename... Args>
    static std::shared_ptr<ProgressBar> create(Args &&...args)
    {
      return std::make_shared<ProgressBar>(std::forward<Args>(args)...);
    }

    std::shared_ptr<ProgressBar> setValue(int val);
  };

} // namespace mui