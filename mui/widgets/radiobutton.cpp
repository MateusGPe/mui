#include "radiobutton.hpp"
#include "../core/scoped.hpp"
#include "app.hpp"
#include <imgui.h>

namespace mui
{
    RadioButton::RadioButton(const std::string &text) : text(text)
    {
        App::assertMainThread();
    }

    void RadioButton::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        bool changed = false;
        if (m_group_selection)
        {
            // For a group, ImGui::RadioButton returns true only for the one that was
            // clicked.
            changed = ImGui::RadioButton(text.c_str(), m_group_selection.get(),
                                         m_button_value);
        }
        else
        {
            // For a standalone button, it toggles the boolean.
            changed = ImGui::RadioButton(text.c_str(), &m_checked);
        }

        if (changed)
        {
            // The callback expects a boolean indicating the new state.
            // Since main.cpp only checks for `if(checked)`, we only need to fire on the
            // positive edge.
            onToggledSignal(isChecked());
        }

        renderTooltip();
        ImGui::EndDisabled();
    }

    bool RadioButton::isChecked() const
    {
        if (m_group_selection)
        {
            return *m_group_selection == m_button_value;
        }
        return m_checked;
    }

    RadioButtonPtr RadioButton::setChecked(bool c)
    {
        if (m_group_selection)
        {
            if (c)
            {
                *m_group_selection = m_button_value;
            }
        }
        else
        {
            m_checked = c;
        }
        return self();
    }

    RadioButtonPtr RadioButton::onToggled(std::function<void(bool)> cb)
    {
        if (cb)
            m_connections.push_back(onToggledSignal.connect(std::move(cb)));
        return self();
    }

    void RadioButton::group(std::initializer_list<RadioButtonPtr> buttons)
    {
        auto selection = std::make_shared<int>(-1);
        int i = 0;
        for (const auto &btn_ptr : buttons)
        {
            btn_ptr->_setGroup(selection, i++);
        }
    }

    void RadioButton::_setGroup(std::shared_ptr<int> selection, int value)
    {
        m_group_selection = selection;
        m_button_value = value;
    }

} // namespace mui
