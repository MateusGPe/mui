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
        EntryPtr self() { return std::static_pointer_cast<Entry>(shared_from_this()); }

        void render() override;
        std::string getText() const;
        EntryPtr setText(const std::string &text);
        EntryPtr setReadOnly(bool readOnly);
        EntryPtr onChanged(std::function<void()> cb);
    };

    class PasswordEntry : public Entry
    {
    public:
        PasswordEntry();
    };

    class SearchEntry : public Entry
    {
    public:
        SearchEntry();
    };
} // namespace mui
