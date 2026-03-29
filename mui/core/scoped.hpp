// core/scoped.hpp
#pragma once
#include <imgui.h>

namespace mui
{
    // --- Scoped Style Colors ---
    class ScopedColor
    {
    private:
        int m_count = 0;

    public:
        ScopedColor() = default;
        ScopedColor(ImGuiCol idx, ImU32 col) { push(idx, col); }
        ScopedColor(ImGuiCol idx, const ImVec4 &col) { push(idx, col); }

        ~ScopedColor()
        {
            if (m_count > 0)
                ImGui::PopStyleColor(m_count);
        }

        // Prevent copying to avoid double-pops
        ScopedColor(const ScopedColor &) = delete;
        ScopedColor &operator=(const ScopedColor &) = delete;

        void push(ImGuiCol idx, ImU32 col)
        {
            ImGui::PushStyleColor(idx, col);
            m_count++;
        }
        void push(ImGuiCol idx, const ImVec4 &col)
        {
            ImGui::PushStyleColor(idx, col);
            m_count++;
        }
    };

    // --- Scoped Style Variables ---
    class ScopedStyle
    {
    private:
        int m_count = 0;

    public:
        ScopedStyle() = default;
        ScopedStyle(ImGuiStyleVar idx, float val) { push(idx, val); }
        ScopedStyle(ImGuiStyleVar idx, const ImVec2 &val) { push(idx, val); }

        ~ScopedStyle()
        {
            if (m_count > 0)
                ImGui::PopStyleVar(m_count);
        }

        ScopedStyle(const ScopedStyle &) = delete;
        ScopedStyle &operator=(const ScopedStyle &) = delete;

        void push(ImGuiStyleVar idx, float val)
        {
            ImGui::PushStyleVar(idx, val);
            m_count++;
        }
        void push(ImGuiStyleVar idx, const ImVec2 &val)
        {
            ImGui::PushStyleVar(idx, val);
            m_count++;
        }
    };

    // --- Scoped ID ---
    class ScopedID
    {
    private:
        int m_count = 0;

    public:
        ScopedID() = default;
        ScopedID(int id) { push(id); }
        ScopedID(const char *str_id) { push(str_id); }
        ScopedID(const std::string &str_id) { push(str_id); }

        ~ScopedID()
        {
            for (int i = 0; i < m_count; ++i)
                ImGui::PopID();
        }

        ScopedID(const ScopedID &) = delete;
        ScopedID &operator=(const ScopedID &) = delete;

        void push(int id) {
            ImGui::PushID(id);
            m_count++;
        }
        void push(const char *str_id) {
            ImGui::PushID(str_id);
            m_count++;
        }
        void push(const std::string &str_id) {
            ImGui::PushID(str_id.c_str());
            m_count++;
        }
    };

    // --- Scoped Item Width ---
    class ScopedItemWidth
    {
    private:
        int m_count = 0;

    public:
        ScopedItemWidth() = default;
        ScopedItemWidth(float item_width) { push(item_width); }
        ~ScopedItemWidth()
        {
            for (int i = 0; i < m_count; ++i)
                ImGui::PopItemWidth();
        }
        void push(float item_width)
        {
            ImGui::PushItemWidth(item_width);
            m_count++;
        }

        ScopedItemWidth(const ScopedItemWidth &) = delete;
        ScopedItemWidth &operator=(const ScopedItemWidth &) = delete;
    };
} // namespace mui