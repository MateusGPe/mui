// widgets/entry.hpp
#pragma once
#include "control.hpp"
#include "../core/observable.hpp"
#include "../core/signal.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace mui
{
    class Entry;
    using EntryPtr = std::shared_ptr<Entry>;

    class Entry : public Control<Entry>
    {
    protected:
        std::string text;
        char *buffer = nullptr;
        size_t bufferSize = 0;
        ImGuiInputTextFlags m_flags;
    
        std::string hint;
        bool isMultiline = false;
        bool withContextMenu = true;
    
        int selStart = 0;
        int selEnd = 0;

		Entry(const std::string &initialText = "", bool password = false, bool multiline = false, float h = 0.0f);
        Entry(char *buf, size_t buf_size);
    
    public:
        mui::Signal<std::string> onChangedSignal;
        mui::Signal<std::string> onEnterSignal;
    
        static EntryPtr create(const std::string &initialText = "", bool password = false, bool multiline = false, float h = 0.0f)
        {
            return std::shared_ptr<Entry>(new Entry(initialText, password, multiline, h));
        }
    
        static EntryPtr create(char *buf, size_t buf_size)
        {
            return std::shared_ptr<Entry>(new Entry(buf, buf_size));
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
        EntryPtr setWithContextMenu(bool c);
        EntryPtr setUseContainerWidth(bool use);
        
        EntryPtr setCharsDecimal(bool b);
        EntryPtr setCharsHexadecimal(bool b);
        EntryPtr setCharsUppercase(bool b);
        EntryPtr setAllowTabInput(bool b);
        EntryPtr setCtrlEnterForNewLine(bool b);
    
        EntryPtr bind(std::shared_ptr<Observable<std::string>> observable);
    
        // Backward compatibility
        EntryPtr onChanged(std::function<void()> cb);
        EntryPtr onEnter(std::function<void(const std::string &)> cb);
    };
    class PasswordEntry;
    using PasswordEntryPtr = std::shared_ptr<PasswordEntry>;

    class PasswordEntry : public Entry
    {
    protected:
        PasswordEntry();
    public:
        static PasswordEntryPtr create() { return std::shared_ptr<PasswordEntry>(new PasswordEntry()); }
    };

    class SearchEntry;
    using SearchEntryPtr = std::shared_ptr<SearchEntry>;

    class SearchEntry : public Entry
    {
    protected:
        SearchEntry();
    public:
        static SearchEntryPtr create() { return std::shared_ptr<SearchEntry>(new SearchEntry()); }
    };
} // namespace mui
