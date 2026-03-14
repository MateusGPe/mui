#pragma once
#include "control.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <ui.h>

namespace mui
{
  class Tab;
  using TabPtr = std::shared_ptr<Tab>;

  class Tab : public Control
  {
  private:
    uiTab *tab;
    std::vector<ControlPtr> children;
    std::function<void()> onSelectedCb;
    static void onSelectedStub(uiTab *t, void *data);

  protected:
    void onHandleDestroyed() override;

  public:
    Tab();

    template <typename... Args>
    static TabPtr create(Args &&...args)
    {
      return std::make_shared<Tab>(std::forward<Args>(args)...);
    }

    TabPtr append(const std::string &name,
                  ControlPtr child);
    TabPtr insertAt(const std::string &name, int index,
                    ControlPtr child);
    TabPtr deletePage(int index);
    int getNumPages() const;
    int getSelected() const;
    TabPtr setSelected(int index);
    TabPtr onSelected(std::function<void()> cb);
    bool getMargined(int page) const;
    TabPtr setMargined(int page, bool margined);
  };

} // namespace mui