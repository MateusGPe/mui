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

    class ComboBox : public Control
    {
    protected:
        std::vector<std::string> items;
        int selectedIndex;
        bool useContainerWidth = false;
        std::function<void()> onChangedCb;

        ComboBoxPtr self() { return std::static_pointer_cast<ComboBox>(shared_from_this()); }

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
        ComboBoxPtr setUseContainerWidth(bool use);
        ComboBoxPtr setSpanAvailWidth(bool span);
    };
} // namespace mui