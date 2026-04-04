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
    protected:
        std::string getTypeName() const override { return "Group"; }
        std::string title;
        IControlPtr child;
        bool margined = false;
        bool m_open = true;
        bool was_collapsed = false;
        bool *show_close_button = nullptr;
        int m_flags = 0;

        Group(const std::string &title);

    public:
        mui::Signal<bool> onToggledSignal;

        static GroupPtr create(const std::string &title) { return std::shared_ptr<Group>(new Group(title)); }

        void renderControl() override;
        GroupPtr setChild(IControlPtr c);
        GroupPtr setMargined(bool margined);

        const std::string &getTitle() const;
        GroupPtr setTitle(const std::string &title);
        bool getMargined() const;

        bool isOpen() const;
        bool getShowCloseButton();

        GroupPtr showCloseButton(bool show);
        GroupPtr setOpen(bool open);
        GroupPtr onToggled(std::function<void(bool)> cb);
        GroupPtr bindTitle(std::shared_ptr<Observable<std::string>> observable);
        GroupPtr bindOpen(std::shared_ptr<Observable<bool>> observable);
        GroupPtr defaultOpen(bool open);
        GroupPtr setAllowOverlap(bool allow);
        GroupPtr setOpenOnDoubleClick(bool open);
        GroupPtr setOpenOnArrow(bool open);
        GroupPtr setBullet(bool show);
        GroupPtr setFramePadding(bool use);
        GroupPtr setSpanAvailWidth(bool span);
        GroupPtr setSpanFullWidth(bool span);
        GroupPtr setNavLeftJumpsToParent(bool allow);

        // ImGuiTreeNodeFlags additions
        GroupPtr setFramed(bool b);
        GroupPtr setSelected(bool b);
        GroupPtr setNoTreePushOnOpen(bool b);
        GroupPtr setNoAutoOpenOnLog(bool b);
        GroupPtr setLeaf(bool b);
        GroupPtr setSpanAllColumns(bool b);
    };
} // namespace mui
