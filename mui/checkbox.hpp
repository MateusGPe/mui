#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{

  class Checkbox;
  using CheckboxPtr = std::shared_ptr<Checkbox>;

  class Checkbox : public Control
  {
    uiCheckbox *checkbox;
    std::function<void()> onToggledCb;
    static void onToggledStub(uiCheckbox *c, void *data);

  public:
    Checkbox(const std::string &text);

    template <typename... Args>
    static CheckboxPtr create(Args &&...args)
    {
      return std::make_shared<Checkbox>(std::forward<Args>(args)...);
    }

    bool isChecked() const;
    std::string getText() const;
    void setText(const std::string &text);
    CheckboxPtr setChecked(bool checked);
    CheckboxPtr onToggled(std::function<void()> cb);
  };

} // namespace mui