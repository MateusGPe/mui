#include "box.hpp"
#include "app.hpp"

namespace mui
{

  // --- Containers ---
  Box::Box(uiBox *b) : box(b) { handle = uiControl(b); }

  void Box::onHandleDestroyed()
  {
    Control::onHandleDestroyed();
    box = nullptr;
    for (auto &c : children)
      c->onHandleDestroyed();
  }

  BoxPtr Box::append(ControlPtr child, bool stretchy)
  {
    verifyState();
    children.push_back(child);
    uiBoxAppend(box, child->getHandle(), stretchy ? 1 : 0);
    child->releaseOwnership();
    return self();
  }

  BoxPtr Box::deleteChild(int index)
  {
    verifyState();
    if (index >= 0 && index < children.size())
    {
      children[index]->acquireOwnership(); // Re-claim ownership since it is
                                           // removed from libui
      uiBoxDelete(box, index);
      children.erase(children.begin() + index);
    }
    return self();
  }

  BoxPtr Box::setPadded(bool padded)
  {
    verifyState();
    uiBoxSetPadded(box, padded ? 1 : 0);
    return self();
  }

  int Box::getNumChildren() const
  {
    verifyState();
    return uiBoxNumChildren(box);
  }

  bool Box::getPadded() const
  {
    verifyState();
    return uiBoxPadded(box);
  }

  VBox::VBox() : Box(uiNewVerticalBox()) { App::assertMainThread(); }
  HBox::HBox() : Box(uiNewHorizontalBox()) { App::assertMainThread(); }

} // namespace mui