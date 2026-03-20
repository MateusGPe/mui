#pragma once
#include "../widgets/control.hpp"
#include <string>

namespace mui
{
    class Group;
    using GroupPtr = std::shared_ptr<Group>;

    class Group : public Control<Group>
    {
        std::string title;
        IControlPtr child;
        bool margined = false;

    public:
        Group(const std::string &title);
        static GroupPtr create(const std::string &title) { return std::make_shared<Group>(title); }

        void renderControl() override;
        GroupPtr setChild(IControlPtr c);
        GroupPtr setMargined(bool margined);
        std::string getTitle() const;
        GroupPtr setTitle(const std::string &title);
        bool getMargined() const;
    };
} // namespace mui
