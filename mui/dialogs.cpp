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
    // RAII for char array return from openFile
    std::unique_ptr<char, decltype(&uiFreeText)> res(
        uiOpenFile(uiWindow(parent.getHandle())), uiFreeText);
    return res ? std::string(res.get()) : std::string();
  }

} // namespace mui