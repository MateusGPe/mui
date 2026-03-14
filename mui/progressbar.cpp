#include "progressbar.hpp"
#include "app.hpp"

namespace mui
{

  ProgressBar::ProgressBar()
  {
    App::assertMainThread();
    progress = uiNewProgressBar();
    handle = uiControl(progress);
  }

  ProgressBarPtr ProgressBar::setValue(int val)
  {
    verifyState();
    uiProgressBarSetValue(progress, val);
    return std::static_pointer_cast<ProgressBar>(shared_from_this());
  }

} // namespace mui