#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{

  class Button : public Control
  {
    uiButton *btn;
    std::function<void()> onClickCb;
    static void onClickedStub(uiButton *b, void *data);

  public:
    Button(const std::string &text);

    template <typename... Args>
    static std::shared_ptr<Button> create(Args &&...args)
    {
      return std::make_shared<Button>(std::forward<Args>(args)...);
    }

    std::string getText() const;
    void setText(const std::string &text);

    std::shared_ptr<Button> onClick(std::function<void()> cb);
  };

} // namespace mui