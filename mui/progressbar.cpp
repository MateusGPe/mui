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
    return self();
  }

} // namespace mui