#include "tab.hpp"
#include "app.hpp"

namespace mui
{

  Tab::Tab()
  {
    App::assertMainThread();
    tab = uiNewTab();
    handle = uiControl(tab);
    uiTabOnSelected(tab, onSelectedStub, this);
  }

  void Tab::onHandleDestroyed()
  {
    Control::onHandleDestroyed();
    tab = nullptr;
    for (auto &c : children)
      c->onHandleDestroyed();
  }

  TabPtr Tab::append(const std::string &name, ControlPtr child)
  {
    verifyState();
    children.push_back(child);
    uiTabAppend(tab, name.c_str(), child->getHandle());
    child->releaseOwnership();
    return std::static_pointer_cast<Tab>(shared_from_this());
  }

  TabPtr Tab::setMargined(int page, bool margined)
  {
    verifyState();
    uiTabSetMargined(tab, page, margined ? 1 : 0);
    return std::static_pointer_cast<Tab>(shared_from_this());
  }

  void Tab::onSelectedStub(uiTab *t, void *data)
  {
    auto self = static_cast<Tab *>(data);
    if (self->onSelectedCb)
      self->onSelectedCb();
  }

  TabPtr Tab::onSelected(std::function<void()> cb)
  {
    verifyState();
    onSelectedCb = std::move(cb);
    return std::static_pointer_cast<Tab>(shared_from_this());
  }

  TabPtr Tab::insertAt(const std::string &name, int index,
                       ControlPtr child)
  {
    verifyState();
    children.insert(children.begin() + index, child);
    uiTabInsertAt(tab, name.c_str(), index, child->getHandle());
    child->releaseOwnership();
    return std::static_pointer_cast<Tab>(shared_from_this());
  }

  TabPtr Tab::deletePage(int index)
  {
    verifyState();
    if (index >= 0 && index < children.size())
    {
      children[index]->acquireOwnership();
      uiTabDelete(tab, index);
      children.erase(children.begin() + index);
    }
    return std::static_pointer_cast<Tab>(shared_from_this());
  }

  int Tab::getNumPages() const
  {
    verifyState();
    return uiTabNumPages(tab);
  }

  int Tab::getSelected() const
  {
    verifyState();
    return uiTabSelected(tab);
  }

  TabPtr Tab::setSelected(int index)
  {
    verifyState();
    uiTabSetSelected(tab, index);
    return std::static_pointer_cast<Tab>(shared_from_this());
  }

  bool Tab::getMargined(int page) const
  {
    verifyState();
    return uiTabMargined(tab, page);
  }

} // namespace mui