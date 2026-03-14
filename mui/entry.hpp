#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{

  class Entry : public Control
  {
    uiEntry *entry;
    std::function<void()> onChangedCb;
    static void onChangedStub(uiEntry *e, void *data);

  protected:
    Entry(uiEntry *e);

  public:
    Entry();

    template <typename... Args>
    static std::shared_ptr<Entry> create(Args &&...args)
    {
      return std::make_shared<Entry>(std::forward<Args>(args)...);
    }

    std::string getText() const;
    std::shared_ptr<Entry> setText(const std::string &text);
    std::shared_ptr<Entry> setReadOnly(bool readOnly);
    bool isReadOnly() const;
    std::shared_ptr<Entry> onChanged(std::function<void()> cb);
  };

  class PasswordEntry : public Entry
  {
  public:
    PasswordEntry();

    template <typename... Args>
    static std::shared_ptr<PasswordEntry> create(Args &&...args)
    {
      return std::make_shared<PasswordEntry>(std::forward<Args>(args)...);
    }
  };

  class SearchEntry : public Entry
  {
  public:
    SearchEntry();

    template <typename... Args>
    static std::shared_ptr<SearchEntry> create(Args &&...args)
    {
      return std::make_shared<SearchEntry>(std::forward<Args>(args)...);
    }
  };

} // namespace mui