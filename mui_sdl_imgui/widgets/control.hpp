#pragma once
#include <memory>
#include <stdexcept>

namespace mui
{
    class Control;
    using ControlPtr = std::shared_ptr<Control>;
    // Helper struct to allow appending controls with an optional 'stretchy' flag.
    struct AppendedControl
    {
        std::shared_ptr<Control> control;
        bool stretchy;

        // Constructor for {control, stretchy} syntax
        inline AppendedControl(std::shared_ptr<Control> c, bool s) : control(std::move(c)), stretchy(s) {}
        // Constructor for {control} syntax, defaulting stretchy to false
        inline AppendedControl(std::shared_ptr<Control> c) : control(std::move(c)), stretchy(false) {}
    };

    // Helper function to append multiple controls to a Box container (VBox, HBox).
    template <typename T>
    inline static void append_all(const std::shared_ptr<T> &container, std::initializer_list<AppendedControl> items)
    {
        for (const auto &item : items)
        {
            container->append(item.control, item.stretchy);
        }
    }
    class Control : public std::enable_shared_from_this<Control>
    {
    protected:
        bool visible = true;
        bool enabled = true;
        bool ownsHandle = false;

    public:
        virtual ~Control();
        virtual void render() = 0;

        void verifyState() const;
        void onHandleDestroyed();
        void show();
        void hide();
        void setEnabled(bool enabled);
        bool isEnabled() const;
        void releaseOwnership();
        void acquireOwnership();
    };
} // namespace mui
