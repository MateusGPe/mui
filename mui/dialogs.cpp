#include "dialogs.hpp"
#include "control.hpp"
#include <memory>
#include <ui.h>

namespace mui
{

  // --- Dialogs (Static Helpers) ---
  void Dialogs::msgBox(Control &parent, const std::string &title,
                       const std::string &description)
  {
    parent.verifyState();
    uiMsgBox(uiWindow(parent.getHandle()), title.c_str(), description.c_str());
  }

  void Dialogs::msgBoxError(Control &parent, const std::string &title,
                            const std::string &description)
  {
    parent.verifyState();
    uiMsgBoxError(uiWindow(parent.getHandle()), title.c_str(),
                  description.c_str());
  }

  std::string Dialogs::openFile(Control &parent)
  {
    parent.verifyState();
    return UiText(uiOpenFile(uiWindow(parent.getHandle())));
  }

} // namespace mui