#pragma once
#include "../widgets/control.hpp"
#include <string>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class Group;
    using GroupPtr = std::shared_ptr<Group>;

    class Group : public Control<Group>
    {
        std::string title;
        IControlPtr child;
        bool margined = false;
        bool m_open = true;

    public:
        mui::Signal<bool> onToggledSignal;

        Group(const std::string &title);
        static GroupPtr create(const std::string &title) { return std::make_shared<Group>(title); }

        void renderControl() override;
        GroupPtr setChild(IControlPtr c);
        GroupPtr setMargined(bool margined);

        std::string getTitle() const;
        GroupPtr setTitle(const std::string &title);
        bool getMargined() const;

        bool isOpen() const;
        GroupPtr setOpen(bool open);
        GroupPtr onToggled(std::function<void(bool)> cb);
        GroupPtr bindTitle(std::shared_ptr<Observable<std::string>> observable);
        GroupPtr bindOpen(std::shared_ptr<Observable<bool>> observable);
    };
} // namespace mui
