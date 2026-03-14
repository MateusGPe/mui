#include "control.hpp"
#include "app.hpp"
#include <stdexcept>
#include <ui.h>

namespace mui
{

  // --- Base Control Class ---
  Control::~Control()
  {
    if (handle && ownsHandle)
    {
      uiControlDestroy(handle);
    }
  }

  void Control::onHandleDestroyed()
  {
    handle = nullptr;
    ownsHandle = false;
  }

  void Control::verifyState() const
  {
    if (!handle)
    {
      throw std::runtime_error("MUI Error: Control handle has been destroyed.");
    }
    App::assertMainThread();
  }

  uiControl *Control::getHandle() const { return handle; }
  void Control::show()
  {
    verifyState();
    uiControlShow(handle);
  }

  void Control::hide()
  {
    verifyState();
    uiControlHide(handle);
  }
  void Control::setEnabled(bool enabled)
  {
    verifyState();
    enabled ? uiControlEnable(handle) : uiControlDisable(handle);
  }
  bool Control::isEnabled() const
  {
    verifyState();
    return uiControlEnabled(handle) != 0;
  }

  void Control::releaseOwnership() { ownsHandle = false; }
  void Control::acquireOwnership() { ownsHandle = true; }

} // namespace mui