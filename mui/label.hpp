#pragma once
#include "control.hpp"
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{

  class Label;
  using LabelPtr = std::shared_ptr<Label>;

  class Label : public Control
  {
    uiLabel *label;

  public:
    Label(const std::string &text);

    template <typename... Args>
    static LabelPtr create(Args &&...args)
    {
      return std::make_shared<Label>(std::forward<Args>(args)...);
    }

    LabelPtr setText(const std::string &text);
    std::string getText() const;
  };

} // namespace mui