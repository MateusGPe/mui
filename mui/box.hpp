#pragma once
#include "control.hpp"
#include <memory>
#include <vector>
#include <ui.h>

namespace mui
{

  class Box;
  using BoxPtr = std::shared_ptr<Box>;
  class VBox;
  using VBoxPtr = std::shared_ptr<VBox>;
  class HBox;
  using HBoxPtr = std::shared_ptr<HBox>;

  // --- Containers ---
  class Box : public Control
  {
  protected:
    uiBox *box;
    std::vector<ControlPtr> children; // Keeps children alive
    Box(uiBox *b);

    void onHandleDestroyed() override;

  public:
    BoxPtr append(ControlPtr child,
                  bool stretchy = false);
    BoxPtr deleteChild(int index);
    BoxPtr setPadded(bool padded);
    int getNumChildren() const;
    bool getPadded() const;
  };

  class VBox : public Box
  {
  public:
    VBox();

    template <typename... Args>
    static VBoxPtr create(Args &&...args)
    {
      return std::make_shared<VBox>(std::forward<Args>(args)...);
    }
  };

  class HBox : public Box
  {
  public:
    HBox();

    template <typename... Args>
    static HBoxPtr create(Args &&...args)
    {
      return std::make_shared<HBox>(std::forward<Args>(args)...);
    }
  };

} // namespace mui