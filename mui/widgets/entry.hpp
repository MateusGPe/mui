#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>

namespace mui
{
    class Entry;
    using EntryPtr = std::shared_ptr<Entry>;

    class Entry : public Control<Entry>
    {
    protected:
        // String mode
        std::string text;

        // Buffer mode
        char* buffer = nullptr;
        size_t bufferSize = 0;

        std::string hint;
        bool readOnly = false;
        bool isPassword = false;
        bool isMultiline = false;
        bool autoSelectAll = false;
        bool noSpaces = false;
        float height = 0.0f;
        float width = 0.0f;

        std::function<void()> onChangedCb;
        std::function<void(const std::string&)> onEnterCb;

    public:
        // String constructor
        Entry(const std::string &initialText = "", bool password = false, bool multiline = false, float h = 0.0f);
        static EntryPtr create(const std::string &initialText = "", bool password = false, bool multiline = false, float h = 0.0f)
        {
            return std::make_shared<Entry>(initialText, password, multiline, h);
        }

        // Buffer constructor
        Entry(char* buf, size_t buf_size);
        static EntryPtr create(char* buf, size_t buf_size)
        {
            return std::make_shared<Entry>(buf, buf_size);
        }


        void renderControl() override;

        std::string getText() const;
        EntryPtr setText(const std::string &t);
        EntryPtr setHint(const std::string &h);
        EntryPtr setReadOnly(bool r);
        EntryPtr setMultiline(bool m, float h = 0.0f);
        EntryPtr setAutoSelectAll(bool a);
        EntryPtr setNoSpaces(bool n);
        EntryPtr setWidth(float w);
        EntryPtr onChanged(std::function<void()> cb);
        EntryPtr onEnter(std::function<void(const std::string&)> cb);
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
