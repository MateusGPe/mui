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

        GroupPtr self() { return std::static_pointer_cast<Group>(shared_from_this()); }

        void render() override;
        GroupPtr setChild(ControlPtr c);
        GroupPtr setMargined(bool margined);
        std::string getTitle() const;
        GroupPtr setTitle(const std::string &title);
        bool getMargined() const;
    };
} // namespace mui
