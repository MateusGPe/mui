#include "entry.hpp"
#include "app.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace mui
{
    Entry::Entry(const std::string &initialText, bool password, bool multiline, float h) : text(initialText), readOnly(false), isPassword(password), isMultiline(multiline), height(h) { App::assertMainThread(); }

    void Entry::renderControl()
    {
        if (!visible)
            return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled || readOnly);

        ImGuiInputTextFlags flags = 0;
        if (isPassword)
            flags |= ImGuiInputTextFlags_Password;
        if (readOnly)
            flags |= ImGuiInputTextFlags_ReadOnly;
        if (autoSelectAll)
            flags |= ImGuiInputTextFlags_AutoSelectAll;
        if (noSpaces)
            flags |= ImGuiInputTextFlags_CharsNoBlank;

        if (spanAvailWidth)
            ImGui::PushItemWidth(-FLT_MIN);

        bool changed = false;
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
        
        if (spanAvailWidth)
            ImGui::PopItemWidth();

        if (changed && onChangedCb)
            onChangedCb();

        renderTooltip();

        ImGui::EndDisabled();
        ImGui::PopID();
    }

    std::string Entry::getText() const { return text; }
    EntryPtr Entry::setText(const std::string &t)
    {
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

    EntryPtr Entry::setSpanAvailWidth(bool s)
    {
        spanAvailWidth = s;
        return self();
    }

    PasswordEntry::PasswordEntry() : Entry() { isPassword = true; }
    SearchEntry::SearchEntry() : Entry() {}
} // namespace mui
