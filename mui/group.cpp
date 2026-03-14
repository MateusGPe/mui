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
    return self();
  }
  GroupPtr Group::setMargined(bool margined)
  {
    verifyState();
    uiGroupSetMargined(group, margined ? 1 : 0);
    return self();
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
    return self();
  }

  bool Group::getMargined() const
  {
    verifyState();
    return uiGroupMargined(group);
  }

} // namespace mui