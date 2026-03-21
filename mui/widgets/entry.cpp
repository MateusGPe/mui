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
    // String constructor
    Entry::Entry(const std::string &initialText, bool password, bool multiline, float h)
        : text(initialText), isPassword(password), isMultiline(multiline)
    {
        App::assertMainThread();
        height = h;
    }

    // Buffer constructor
    Entry::Entry(char *buf, size_t buf_size)
        : buffer(buf), bufferSize(buf_size)
    {
        App::assertMainThread();
    }

    void Entry::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiInputTextFlags flags = 0;
        if (isPassword)
            flags |= ImGuiInputTextFlags_Password;
        if (readOnly)
            flags |= ImGuiInputTextFlags_ReadOnly;
        if (autoSelectAll)
            flags |= ImGuiInputTextFlags_AutoSelectAll;
        if (noSpaces)
            flags |= ImGuiInputTextFlags_CharsNoBlank;
        if (onEnterCb)
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

            // Decide whether to use buffer or std::string version
            if (buffer)
            {
                if (isMultiline)
                {
                    // Not implemented for buffer mode
                }
                else if (!hint.empty())
                {
                    entered = ImGui::InputTextWithHint("##entry", hint.c_str(), buffer, bufferSize, flags, callback, this);
                }
                else
                {
                    entered = ImGui::InputText("##entry", buffer, bufferSize, flags, callback, this);
                }
                // For buffers, InputText returns true on enter, not change. We can check for edits manually.
                if (ImGui::IsItemEdited())
                    changed = true;
            }
            else // std::string mode
            {
                if (isMultiline)
                {
                    changed = ImGui::InputTextMultiline("##entry", &text, ImVec2(spanAvailWidth ? -FLT_MIN : 0, height), flags, callback, this);
                }
                else if (!hint.empty())
                {
                    changed = ImGui::InputTextWithHint("##entry", hint.c_str(), &text, flags, callback, this);
                }
                else
                {
                    changed = ImGui::InputText("##entry", &text, flags, callback, this);
                }
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

                if (ImGui::MenuItem("Cut", "CTRL+X", false, has_selection && !readOnly))
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
                if (ImGui::MenuItem("Paste", "CTRL+V", false, !readOnly && ImGui::GetClipboardText() != nullptr))
                {
                    const char *clipboard_text = ImGui::GetClipboardText();
                    if (clipboard_text)
                    {
                        std::string new_text = current_text;
                        if (has_selection)
                        {
                            new_text.erase(start, end - start);
                        }

                        std::string clipboard_str = clipboard_text;

                        if (!isMultiline)
                        {
                            if (noSpaces)
                            {
                                clipboard_str.erase(std::remove_if(clipboard_str.begin(), clipboard_str.end(), [](char c)
                                                                   { return c == '\n' || c == '\r' || c == ' ' || c == '\t'; }),
                                                    clipboard_str.end());
                            }
                            else if (isPassword)
                            {
                                clipboard_str.erase(std::remove_if(clipboard_str.begin(), clipboard_str.end(), [](char c)
                                                                   { return c == '\n' || c == '\r'; }),
                                                    clipboard_str.end());
                            }
                            else
                            {
                                std::replace_if(clipboard_str.begin(), clipboard_str.end(), [](char c)
                                                { return c == '\n' || c == '\r'; }, ' ');
                            }
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

                if (ImGui::MenuItem("Clear", nullptr, false, !readOnly && has_text))
                {
                    setText("");
                    changed = true;
                }
                ImGui::EndPopup();
            }

            if (changed && onChangedCb)
                onChangedCb();
            if (entered && onEnterCb)
                onEnterCb(buffer ? std::string(buffer) : text);
        };

        ScopedItemWidth item_width;
        if (width > 0)
        {
            item_width.push(width);
        }
        else if (spanAvailWidth)
        {
            item_width.push(-FLT_MIN);
        }
        else if (useContainerWidth)
        {
            // Do nothing, will inherit from container.
        }
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
        {
            text = t;
        }
        return self();
    }
    EntryPtr Entry::setHint(const std::string &h)
    {
        hint = h;
        return self();
    }
    EntryPtr Entry::setReadOnly(bool r)
    {
        readOnly = r;
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
        autoSelectAll = a;
        return self();
    }
    EntryPtr Entry::setNoSpaces(bool n)
    {
        noSpaces = n;
        return self();
    }
    EntryPtr Entry::onChanged(std::function<void()> cb)
    {
        onChangedCb = std::move(cb);
        return self();
    }
    EntryPtr Entry::onEnter(std::function<void(const std::string &)> cb)
    {
        onEnterCb = std::move(cb);
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

    PasswordEntry::PasswordEntry() : Entry() { isPassword = true; }
    SearchEntry::SearchEntry() : Entry() {}
} // namespace mui
