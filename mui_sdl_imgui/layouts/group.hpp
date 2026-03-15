#pragma once
#include "control.hpp"
#include <string>

namespace mui
{
    class Group;
    using GroupPtr = std::shared_ptr<Group>;

    class Group : public Control
    {
        std::string title;
        ControlPtr child;
        bool margined = false;

    public:
        Group(const std::string &title);
        static GroupPtr create(const std::string &title) { return std::make_shared<Group>(title); }

        GroupPtr self() { return std::static_pointer_cast<Group>(shared_from_this()); }

        void renderControl() override;
        GroupPtr setChild(ControlPtr c);
        GroupPtr setMargined(bool margined);
        std::string getTitle() const;
        GroupPtr setTitle(const std::string &title);
        bool getMargined() const;
    };
} // namespace mui
