// widgets/radiogroup.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mui
{
    class RadioGroup;
    using RadioGroupPtr = std::shared_ptr<RadioGroup>;

    class RadioGroup : public Control
    {
    protected:
        std::vector<std::string> options;
        int selectedIndex;
        bool horizontal;
        std::function<void()> onSelectedCb;

        RadioGroupPtr self() { return std::static_pointer_cast<RadioGroup>(shared_from_this()); }

    public:
        RadioGroup();
        static RadioGroupPtr create() { return std::make_shared<RadioGroup>(); }

        void render() override;

        RadioGroupPtr append(const std::string &option);

        int getSelected() const;
        RadioGroupPtr setSelected(int index);
        RadioGroupPtr setHorizontal(bool h);
        RadioGroupPtr onSelected(std::function<void()> cb);
    };
} // namespace mui