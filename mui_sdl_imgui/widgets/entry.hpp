#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>

namespace mui
{
    class Entry;
    using EntryPtr = std::shared_ptr<Entry>;

    class Entry : public Control
    {
    protected:
        std::string text;
        std::string hint;
        bool readOnly;
        bool isPassword;
        bool isMultiline;
        bool autoSelectAll = false;
        bool noSpaces = false;
        float height;

        std::function<void()> onChangedCb;

        EntryPtr self() { return std::static_pointer_cast<Entry>(shared_from_this()); }

    public:
        Entry(const std::string &initialText = "", bool password = false, bool multiline = false, float h = 0.0f);
        static EntryPtr create(const std::string &initialText = "", bool password = false, bool multiline = false, float h = 0.0f)
        {
            return std::make_shared<Entry>(initialText, password, multiline, h);
        }

        void renderControl() override;

        std::string getText() const;
        EntryPtr setText(const std::string &t);
        EntryPtr setHint(const std::string &h);
        EntryPtr setReadOnly(bool r);
        EntryPtr setMultiline(bool m, float h = 0.0f);
        EntryPtr setAutoSelectAll(bool a);
        EntryPtr setNoSpaces(bool n);
        EntryPtr setSpanAvailWidth(bool s);
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
