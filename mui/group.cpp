#include "group.hpp"
#include "app.hpp"

namespace mui
{

  Group::Group(const std::string &title)
  {
    App::assertMainThread();
    group = uiNewGroup(title.c_str());
    handle = uiControl(group);
  }
  void Group::onHandleDestroyed()
  {
    Control::onHandleDestroyed();
    group = nullptr;
    if (child)
      child->onHandleDestroyed();
  }
  GroupPtr Group::setChild(ControlPtr c)
  {
    verifyState();
    child = c;
    uiGroupSetChild(group, c->getHandle());
    c->releaseOwnership();
    return std::static_pointer_cast<Group>(shared_from_this());
  }
  GroupPtr Group::setMargined(bool margined)
  {
    verifyState();
    uiGroupSetMargined(group, margined ? 1 : 0);
    return std::static_pointer_cast<Group>(shared_from_this());
  }

  std::string Group::getTitle() const
  {
    verifyState();
    return UiText(uiGroupTitle(group));
  }

  GroupPtr Group::setTitle(const std::string &title)
  {
    verifyState();
    uiGroupSetTitle(group, title.c_str());
    return std::static_pointer_cast<Group>(shared_from_this());
  }

  bool Group::getMargined() const
  {
    verifyState();
    return uiGroupMargined(group);
  }

} // namespace mui