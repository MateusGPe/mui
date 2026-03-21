#include "entry.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "../core/scoped.hpp"

namespace mui
{
    // String constructor
    Entry::Entry(const std::string &initialText, bool password, bool multiline, float h)
        : text(initialText), isPassword(password), isMultiline(multiline), height(h)
    {
        App::assertMainThread();
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
                    entered = ImGui::InputTextWithHint("##entry", hint.c_str(), buffer, bufferSize, flags);
                }
                else
                {
                    entered = ImGui::InputText("##entry", buffer, bufferSize, flags);
                }
                // For buffers, InputText returns true on enter, not change. We can check for edits manually.
                if (ImGui::IsItemEdited())
                    changed = true;
            }
            else // std::string mode
            {
                if (isMultiline)
                {
                    changed = ImGui::InputTextMultiline("##entry", &text, ImVec2(spanAvailWidth ? -FLT_MIN : 0, height), flags);
                }
                else if (!hint.empty())
                {
                    changed = ImGui::InputTextWithHint("##entry", hint.c_str(), &text, flags);
                }
                else
                {
                    changed = ImGui::InputText("##entry", &text, flags);
                }
                if (ImGui::IsItemDeactivatedAfterEdit() && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)))
                {
                    entered = true;
                }
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

    PasswordEntry::PasswordEntry() : Entry() { isPassword = true; }
    SearchEntry::SearchEntry() : Entry() {}
} // namespace mui
