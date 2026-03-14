#pragma once
#include "control.hpp"
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{

  class Group : public Control
  {
  private:
    uiGroup *group;
    std::shared_ptr<Control> child;

  protected:
    void onHandleDestroyed() override;

  public:
    Group(const std::string &title);

    template <typename... Args>
    static std::shared_ptr<Group> create(Args &&...args)
    {
      return std::make_shared<Group>(std::forward<Args>(args)...);
    }

    std::shared_ptr<Group> setChild(std::shared_ptr<Control> child);
    std::shared_ptr<Group> setMargined(bool margined);
    std::string getTitle() const;
    std::shared_ptr<Group> setTitle(const std::string &title);
    bool getMargined() const;
  };

} // namespace mui