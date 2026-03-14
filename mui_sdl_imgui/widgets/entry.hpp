#pragma once
#include "control.hpp"
#include <string>
#include <functional>

namespace mui
{
    class Entry;
    using EntryPtr = std::shared_ptr<Entry>;

    class Entry : public Control
    {
    protected:
        std::string text;
        bool readOnly;
        bool isPassword = false;
        std::function<void()> onChangedCb;

    public:
        Entry();
        static EntryPtr create() { return std::make_shared<Entry>(); }
        EntryPtr self() { return std::static_pointer_cast<Entry>(shared_from_this()); }

        void render() override;
        std::string getText() const;
        EntryPtr setText(const std::string &text);
        EntryPtr setReadOnly(bool readOnly);
        EntryPtr onChanged(std::function<void()> cb);
    };

    class PasswordEntry;
    using PasswordEntryPtr = std::shared_ptr<PasswordEntry>;

    class PasswordEntry : public Entry
    {
    public:
        PasswordEntry();
        static PasswordEntryPtr create() { return std::make_shared<PasswordEntry>(); }
    };

    class SearchEntry;
    using SearchEntryPtr = std::shared_ptr<SearchEntry>;

    class SearchEntry : public Entry
    {
    public:
        SearchEntry();
        static SearchEntryPtr create() { return std::make_shared<SearchEntry>(); }
    };
} // namespace mui
