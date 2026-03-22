// widgets/entry.cpp
#include "entry.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>
#include "imstb_textedit.h"
#include "../core/scoped.hpp"
#include <algorithm>

namespace mui
{
    Entry::Entry(const std::string &initialText, bool password, bool multiline, float h)
        : text(initialText), isMultiline(multiline)
    {
        App::assertMainThread();
        height = h;
        m_flags = 0;
        if (password)
            m_flags |= ImGuiInputTextFlags_Password;
    }

    Entry::Entry(char *buf, size_t buf_size)
        : buffer(buf), bufferSize(buf_size)
    {
        App::assertMainThread();
        m_flags = 0;
    }

    void Entry::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiInputTextFlags flags = m_flags;
        if (onEnterSignal.slot_count() > 0)
            flags |= ImGuiInputTextFlags_EnterReturnsTrue;

        ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData *data) -> int
        {
            Entry *entry = (Entry *)data->UserData;
            if (entry)
            {
                entry->selStart = data->SelectionStart;
                entry->selEnd = data->SelectionEnd;
            }
            return 0;
        };
        flags |= ImGuiInputTextFlags_CallbackAlways;

        auto render_entry = [&]
        {
            bool changed = false;
            bool entered = false;

            if (buffer)
            {
                if (isMultiline)
                { /* Not implemented for buffer mode */
                }
                else if (!hint.empty())
                    entered = ImGui::InputTextWithHint("##entry", hint.c_str(), buffer, bufferSize, flags, callback, this);
                else
                    entered = ImGui::InputText("##entry", buffer, bufferSize, flags, callback, this);
                if (ImGui::IsItemEdited())
                    changed = true;
            }
            else
            {
                if (isMultiline)
                    changed = ImGui::InputTextMultiline("##entry", &text, ImVec2(spanAvailWidth ? -FLT_MIN : 0, height), flags, callback, this);
                else if (!hint.empty())
                    changed = ImGui::InputTextWithHint("##entry", hint.c_str(), &text, flags, callback, this);
                else
                    changed = ImGui::InputText("##entry", &text, flags, callback, this);

                if (ImGui::IsItemDeactivatedAfterEdit() && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)))
                {
                    entered = true;
                }
            }

            if (withContextMenu && ImGui::BeginPopupContextItem("##context_menu"))
            {
                std::string current_text = getText();
                bool has_text = !current_text.empty();
                int start = selStart;
                int end = selEnd;
                if (start > end)
                    std::swap(start, end);
                bool has_selection = (start != end);

                if (ImGui::MenuItem("Cut", "CTRL+X", false, has_selection && !(m_flags & ImGuiInputTextFlags_ReadOnly)))
                {
                    std::string selected_text = current_text.substr(start, end - start);
                    ImGui::SetClipboardText(selected_text.c_str());
                    std::string new_text = current_text;
                    new_text.erase(start, end - start);
                    setText(new_text);
                    changed = true;
                }
                if (ImGui::MenuItem("Copy", "CTRL+C", false, has_selection))
                {
                    std::string selected_text = current_text.substr(start, end - start);
                    ImGui::SetClipboardText(selected_text.c_str());
                }
                if (ImGui::MenuItem("Paste", "CTRL+V", false, !(m_flags & ImGuiInputTextFlags_ReadOnly) && ImGui::GetClipboardText() != nullptr))
                {
                    const char *clipboard_text = ImGui::GetClipboardText();
                    if (clipboard_text)
                    {
                        std::string new_text = current_text;
                        if (has_selection)
                            new_text.erase(start, end - start);
                        std::string clipboard_str = clipboard_text;

                        if (!isMultiline)
                        {
                            if (m_flags & ImGuiInputTextFlags_CharsNoBlank)
                                clipboard_str.erase(std::remove_if(clipboard_str.begin(), clipboard_str.end(), [](char c)
                                                                   { return c == '\n' || c == '\r' || c == ' ' || c == '\t'; }),
                                                    clipboard_str.end());
                            else if (m_flags & ImGuiInputTextFlags_Password)
                                clipboard_str.erase(std::remove_if(clipboard_str.begin(), clipboard_str.end(), [](char c)
                                                                   { return c == '\n' || c == '\r'; }),
                                                    clipboard_str.end());
                            else
                                std::replace_if(clipboard_str.begin(), clipboard_str.end(), [](char c)
                                                { return c == '\n' || c == '\r'; }, ' ');
                        }

                        int insert_pos = start;
                        if (insert_pos > new_text.length())
                            insert_pos = new_text.length();
                        new_text.insert(insert_pos, clipboard_str);
                        setText(new_text);
                        changed = true;
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Clear", nullptr, false, !(m_flags & ImGuiInputTextFlags_ReadOnly) && has_text))
                {
                    setText("");
                    changed = true;
                }
                ImGui::EndPopup();
            }

            if (changed)
                onChangedSignal(getText());
            if (entered)
                onEnterSignal(getText());
        };

        ScopedItemWidth item_width;
        if (width > 0)
            item_width.push(width);
        else if (spanAvailWidth)
            item_width.push(-FLT_MIN);

        render_entry();
        renderTooltip();
        ImGui::EndDisabled();
    }

    std::string Entry::getText() const
    {
        if (buffer)
            return std::string(buffer);
        return text;
    }

    EntryPtr Entry::setText(const std::string &t)
    {
        if (buffer)
        {
            strncpy(buffer, t.c_str(), bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
        }
        else
            text = t;
        return self();
    }

    EntryPtr Entry::setHint(const std::string &h)
    {
        hint = h;
        return self();
    }
    EntryPtr Entry::setReadOnly(bool r)
    {
        if (r)
            m_flags |= ImGuiInputTextFlags_ReadOnly;
        else
            m_flags &= ~ImGuiInputTextFlags_ReadOnly;
        return self();
    }
    EntryPtr Entry::setMultiline(bool m, float h)
    {
        isMultiline = m;
        height = h;
        return self();
    }
    EntryPtr Entry::setAutoSelectAll(bool a)
    {
        if (a)
            m_flags |= ImGuiInputTextFlags_AutoSelectAll;
        else
            m_flags &= ~ImGuiInputTextFlags_AutoSelectAll;
        return self();
    }
    EntryPtr Entry::setNoSpaces(bool n)
    {
        if (n)
            m_flags |= ImGuiInputTextFlags_CharsNoBlank;
        else
            m_flags &= ~ImGuiInputTextFlags_CharsNoBlank;
        return self();
    }
    EntryPtr Entry::setWidth(float w)
    {
        width = w;
        return self();
    }
    EntryPtr Entry::setWithContextMenu(bool c)
    {
        withContextMenu = c;
        return self();
    }
    EntryPtr Entry::setUseContainerWidth(bool use)
    {
        useContainerWidth = use;
        return self();
    }

    EntryPtr Entry::setCharsDecimal(bool b)
    {
        if (b)
            m_flags |= ImGuiInputTextFlags_CharsDecimal;
        else
            m_flags &= ~ImGuiInputTextFlags_CharsDecimal;
        return self();
    }
    EntryPtr Entry::setCharsHexadecimal(bool b)
    {
        if (b)
            m_flags |= ImGuiInputTextFlags_CharsHexadecimal;
        else
            m_flags &= ~ImGuiInputTextFlags_CharsHexadecimal;
        return self();
    }
    EntryPtr Entry::setCharsUppercase(bool b)
    {
        if (b)
            m_flags |= ImGuiInputTextFlags_CharsUppercase;
        else
            m_flags &= ~ImGuiInputTextFlags_CharsUppercase;
        return self();
    }
    EntryPtr Entry::setAllowTabInput(bool b)
    {
        if (b)
            m_flags |= ImGuiInputTextFlags_AllowTabInput;
        else
            m_flags &= ~ImGuiInputTextFlags_AllowTabInput;
        return self();
    }
    EntryPtr Entry::setCtrlEnterForNewLine(bool b)
    {
        if (b)
            m_flags |= ImGuiInputTextFlags_CtrlEnterForNewLine;
        else
            m_flags &= ~ImGuiInputTextFlags_CtrlEnterForNewLine;
        return self();
    }

    EntryPtr Entry::bind(std::shared_ptr<Observable<std::string>> observable)
    {
        setText(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const std::string &val)
                                                                   { mui::App::queueMain([this, val]()
                                                                                         { this->setText(val); }); }));
        m_connections.push_back(onChangedSignal.connect([observable](const std::string &val)
                                                        { observable->set(val); }));
        return self();
    }

    EntryPtr Entry::onChanged(std::function<void()> cb)
    {
        if (cb)
            m_connections.push_back(onChangedSignal.connect([cb](const std::string &)
                                                            { cb(); }));
        return self();
    }

    EntryPtr Entry::onEnter(std::function<void(const std::string &)> cb)
    {
        if (cb)
            m_connections.push_back(onEnterSignal.connect(std::move(cb)));
        return self();
    }

    PasswordEntry::PasswordEntry() : Entry() { m_flags |= ImGuiInputTextFlags_Password; }
    SearchEntry::SearchEntry() : Entry() {}
} // namespace mui
