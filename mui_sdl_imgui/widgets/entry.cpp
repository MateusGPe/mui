#include "entry.hpp"
#include "app.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace mui
{
    Entry::Entry() : readOnly(false) { App::assertMainThread(); }

    void Entry::render()
    {
        if (!visible) return;
        ImGui::PushID(this);
        ImGui::BeginDisabled(!enabled || readOnly);
        
        ImGuiInputTextFlags flags = isPassword ? ImGuiInputTextFlags_Password : 0;
        if (ImGui::InputText("##entry", &text, flags)) {
            if (onChangedCb) onChangedCb();
        }
        
        ImGui::EndDisabled();
        ImGui::PopID();
    }

    std::string Entry::getText() const { return text; }
    EntryPtr Entry::setText(const std::string &t) { text = t; return self(); }
    EntryPtr Entry::setReadOnly(bool r) { readOnly = r; return self(); }
    EntryPtr Entry::onChanged(std::function<void()> cb) { onChangedCb = std::move(cb); return self(); }

    PasswordEntry::PasswordEntry() : Entry() { isPassword = true; }
    SearchEntry::SearchEntry() : Entry() {} 
} // namespace mui
