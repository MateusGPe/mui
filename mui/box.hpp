#pragma once
#include "control.hpp"
#include <memory>
#include <vector>
#include <ui.h>

namespace mui
{

  // --- Containers ---
  class Box : public Control
  {
  protected:
    uiBox *box;
    std::vector<std::shared_ptr<Control>> children; // Keeps children alive
    Box(uiBox *b);

    void onHandleDestroyed() override;

  public:
    std::shared_ptr<Box> append(std::shared_ptr<Control> child,
                                bool stretchy = false);
    std::shared_ptr<Box> deleteChild(int index);
    std::shared_ptr<Box> setPadded(bool padded);
    int getNumChildren() const;
    bool getPadded() const;
  };

  class VBox : public Box
  {
  public:
    VBox();

    template <typename... Args>
    static std::shared_ptr<VBox> create(Args &&...args)
    {
      return std::make_shared<VBox>(std::forward<Args>(args)...);
    }
  };

  class HBox : public Box
  {
  public:
    HBox();

    template <typename... Args>
    static std::shared_ptr<HBox> create(Args &&...args)
    {
      return std::make_shared<HBox>(std::forward<Args>(args)...);
    }
  };

} // namespace mui