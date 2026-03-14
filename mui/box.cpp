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

  std::shared_ptr<Box> Box::append(std::shared_ptr<Control> child, bool stretchy)
  {
    verifyState();
    children.push_back(child);
    uiBoxAppend(box, child->getHandle(), stretchy ? 1 : 0);
    child->releaseOwnership();
    return std::static_pointer_cast<Box>(shared_from_this());
  }

  std::shared_ptr<Box> Box::deleteChild(int index)
  {
    verifyState();
    if (index >= 0 && index < children.size())
    {
      children[index]->acquireOwnership(); // Re-claim ownership since it is
                                           // removed from libui
      uiBoxDelete(box, index);
      children.erase(children.begin() + index);
    }
    return std::static_pointer_cast<Box>(shared_from_this());
  }

  std::shared_ptr<Box> Box::setPadded(bool padded)
  {
    verifyState();
    uiBoxSetPadded(box, padded ? 1 : 0);
    return std::static_pointer_cast<Box>(shared_from_this());
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