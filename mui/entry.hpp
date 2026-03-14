#pragma once
#include "control.hpp"
#include <functional>
#include <memory>
#include <string>
#include <ui.h>

namespace mui
{

  class Entry;
  using EntryPtr = std::shared_ptr<Entry>;
  class PasswordEntry;
  using PasswordEntryPtr = std::shared_ptr<PasswordEntry>;
  class SearchEntry;
  using SearchEntryPtr = std::shared_ptr<SearchEntry>;

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
    static EntryPtr create(Args &&...args)
    {
      return std::make_shared<Entry>(std::forward<Args>(args)...);
    }

    std::string getText() const;
    EntryPtr setText(const std::string &text);
    EntryPtr setReadOnly(bool readOnly);
    bool isReadOnly() const;
    EntryPtr onChanged(std::function<void()> cb);
  };

  class PasswordEntry : public Entry
  {
  public:
    PasswordEntry();

    template <typename... Args>
    static PasswordEntryPtr create(Args &&...args)
    {
      return std::make_shared<PasswordEntry>(std::forward<Args>(args)...);
    }
  };

  class SearchEntry : public Entry
  {
  public:
    SearchEntry();

    template <typename... Args>
    static SearchEntryPtr create(Args &&...args)
    {
      return std::make_shared<SearchEntry>(std::forward<Args>(args)...);
    }
  };

} // namespace mui