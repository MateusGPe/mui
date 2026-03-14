#include "tab.hpp"
#include "app.hpp"

namespace mui
{

  MUI_IMPL_STATELESS_CB(Tab, uiTab, onSelectedStub, onSelectedCb)

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
    return self();
  }

  TabPtr Tab::setMargined(int page, bool margined)
  {
    verifyState();
    uiTabSetMargined(tab, page, margined ? 1 : 0);
    return self();
  }

  TabPtr Tab::onSelected(std::function<void()> cb)
  {
    verifyState();
    onSelectedCb = std::move(cb);
    return self();
  }

  TabPtr Tab::insertAt(const std::string &name, int index, ControlPtr child)
  {
    verifyState();
    children.insert(children.begin() + index, child);
    uiTabInsertAt(tab, name.c_str(), index, child->getHandle());
    child->releaseOwnership();
    return self();
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
    return self();
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
    return self();
  }

  bool Tab::getMargined(int page) const
  {
    verifyState();
    return uiTabMargined(tab, page);
  }

} // namespace mui