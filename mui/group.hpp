#pragma once
#include "control.hpp"
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{
  class Group;
  using GroupPtr = std::shared_ptr<Group>;

  class Group : public Control
  {
  private:
    uiGroup *group;
    ControlPtr child;

  protected:
    void onHandleDestroyed() override;

  public:
    Group(const std::string &title);

    template <typename... Args>
    static GroupPtr create(Args &&...args)
    {
      return std::make_shared<Group>(std::forward<Args>(args)...);
    }

    GroupPtr setChild(ControlPtr child);
    GroupPtr setMargined(bool margined);
    std::string getTitle() const;
    GroupPtr setTitle(const std::string &title);
    bool getMargined() const;
  };

} // namespace mui