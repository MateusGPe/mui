#include "entry.hpp"
#include "app.hpp"

namespace mui
{

  MUI_IMPL_STATELESS_CB(Entry, uiEntry, onChangedStub, onChangedCb)

  Entry::Entry(uiEntry *e) : entry(e)
  {
    handle = uiControl(entry);
    uiEntryOnChanged(entry, onChangedStub, this);
  }

  Entry::Entry() : Entry(uiNewEntry()) { App::assertMainThread(); }

  std::string Entry::getText() const
  {
    verifyState();
    return UiText(uiEntryText(entry));
  }

  EntryPtr Entry::setText(const std::string &text)
  {
    verifyState();
    uiEntrySetText(entry, text.c_str());
    return self();
  }

  EntryPtr Entry::setReadOnly(bool readOnly)
  {
    verifyState();
    uiEntrySetReadOnly(entry, readOnly ? 1 : 0);
    return self();
  }

  EntryPtr Entry::onChanged(std::function<void()> cb)
  {
    verifyState();
    onChangedCb = std::move(cb);
    return self();
  }

  bool Entry::isReadOnly() const
  {
    verifyState();
    return uiEntryReadOnly(entry);
  }

  PasswordEntry::PasswordEntry() : Entry(uiNewPasswordEntry())
  {
    App::assertMainThread();
  }

  SearchEntry::SearchEntry() : Entry(uiNewSearchEntry())
  {
    App::assertMainThread();
  }

} // namespace mui