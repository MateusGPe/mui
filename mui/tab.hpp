#pragma once
#include "control.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <ui.h>

namespace mui
{

  class Tab : public Control
  {
  private:
    uiTab *tab;
    std::vector<std::shared_ptr<Control>> children;
    std::function<void()> onSelectedCb;
    static void onSelectedStub(uiTab *t, void *data);

  protected:
    void onHandleDestroyed() override;

  public:
    Tab();

    template <typename... Args>
    static std::shared_ptr<Tab> create(Args &&...args)
    {
      return std::make_shared<Tab>(std::forward<Args>(args)...);
    }

    std::shared_ptr<Tab> append(const std::string &name,
                                std::shared_ptr<Control> child);
    std::shared_ptr<Tab> insertAt(const std::string &name, int index,
                                  std::shared_ptr<Control> child);
    std::shared_ptr<Tab> deletePage(int index);
    int getNumPages() const;
    int getSelected() const;
    std::shared_ptr<Tab> setSelected(int index);
    std::shared_ptr<Tab> onSelected(std::function<void()> cb);
    bool getMargined(int page) const;
    std::shared_ptr<Tab> setMargined(int page, bool margined);
  };

} // namespace mui