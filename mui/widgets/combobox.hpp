// widgets/combobox.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class ComboBox;
    using ComboBoxPtr = std::shared_ptr<ComboBox>;

    class ComboBox : public Control<ComboBox>
    {
    protected:
        std::vector<std::string> items;
        int selectedIndex;
        ImGuiComboFlags m_flags;
    
        ComboBox();

    public:
        mui::Signal<int> onChangedSignal;
        
        static ComboBoxPtr create() { return std::shared_ptr<ComboBox>(new ComboBox()); }
    
        void renderControl() override;
    
        ComboBoxPtr append(const std::string &item);
        ComboBoxPtr clear();
    
        int getSelectedIndex() const;
        std::string getText() const;
        ComboBoxPtr setSelectedIndex(int index);
        ComboBoxPtr bind(std::shared_ptr<Observable<int>> observable);
        ComboBoxPtr onChanged(std::function<void(int)> cb);
        ComboBoxPtr setSpanAvailWidth(bool span);
        ComboBoxPtr setMinWidth(float w);
    
        ComboBoxPtr setPopupAlignLeft(bool b);
        ComboBoxPtr setHeightSmall(bool b);
        ComboBoxPtr setHeightRegular(bool b);
        ComboBoxPtr setHeightLarge(bool b);
        ComboBoxPtr setHeightLargest(bool b);
        ComboBoxPtr setNoArrowButton(bool b);
        ComboBoxPtr setNoPreview(bool b);
        ComboBoxPtr setCustomPreview(bool b);
    };
} // namespace mui