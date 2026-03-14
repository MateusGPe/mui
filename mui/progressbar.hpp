#pragma once
#include "control.hpp"
#include <memory>
#include <ui.h>

namespace mui
{

  class ProgressBar;
  using ProgressBarPtr = std::shared_ptr<ProgressBar>;

  class ProgressBar : public Control
  {
    uiProgressBar *progress;

  public:
    ProgressBar();

    template <typename... Args>
    static ProgressBarPtr create(Args &&...args)
    {
      return std::make_shared<ProgressBar>(std::forward<Args>(args)...);
    }

    ProgressBarPtr setValue(int val);
  };

} // namespace mui