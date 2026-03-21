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
        std::function<void()> onChangedCb;

    public:
        mui::Signal<int> onChangedSignal;
        ComboBox();
        static ComboBoxPtr create() { return std::make_shared<ComboBox>(); }

        void renderControl() override;

        ComboBoxPtr append(const std::string &item);
        ComboBoxPtr clear();

        int getSelectedIndex() const;
        std::string getText() const;
        ComboBoxPtr setSelectedIndex(int index);
        ComboBoxPtr bind(std::shared_ptr<Observable<int>> observable);
        ComboBoxPtr onChanged(std::function<void()> cb);
        ComboBoxPtr setSpanAvailWidth(bool span);
        ComboBoxPtr setMinWidth(float w);
    };
} // namespace mui