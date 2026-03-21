// widgets/combobox.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mui
{
    class ComboBox;
    using ComboBoxPtr = std::shared_ptr<ComboBox>;

    class ComboBox : public Control<ComboBox>
    {
    protected:
        std::vector<std::string> items;
        int selectedIndex;
        std::function<void()> onChangedCb;

    public:
        ComboBox();
        static ComboBoxPtr create() { return std::make_shared<ComboBox>(); }

        void renderControl() override;

        ComboBoxPtr append(const std::string &item);
        ComboBoxPtr clear();
        
        int getSelectedIndex() const;
        std::string getText() const;
        ComboBoxPtr setSelectedIndex(int index);
        ComboBoxPtr onChanged(std::function<void()> cb);
        ComboBoxPtr setSpanAvailWidth(bool span);
    };
} // namespace mui