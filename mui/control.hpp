#pragma once
#include <memory>
#include <ui.h>
#include <string>
#include "core.hpp"

namespace mui
{

    // Forward declarations for friend classes
    class Window;
    class Group;
    class Tab;
    class Box;
    class Label;
    class Button;
    class Control;
    using ControlPtr = std::shared_ptr<Control>;

    inline static std::string UiText(char *rawString)
    {
        std::unique_ptr<char, decltype(&uiFreeText)> cstr(rawString, uiFreeText);
        return cstr ? std::string(cstr.get()) : std::string();
    }

    // --- Base Control Class ---
    class Control // : public std::enable_shared_from_this<Control>
    {
    protected:
        uiControl *handle = nullptr;
        bool ownsHandle = true; // Flag for RAII ownership enforcement

        // Traverses the C++ tree to neutralize pointers when the C UI tree is
        // destroyed
        virtual void onHandleDestroyed();

    public:
        Control() = default;
        virtual ~Control();

        // Hard enforce RAII: non-copyable and non-movable
        Control(const Control &) = delete;
        Control &operator=(const Control &) = delete;
        Control(Control &&) = delete;
        Control &operator=(Control &&) = delete;

        uiControl *getHandle() const;
        void show();
        void hide();
        void setEnabled(bool enabled);
        bool isEnabled() const;

        void releaseOwnership();
        void acquireOwnership();
        void verifyState() const;

        friend class Window; // Allow window to trigger onHandleDestroyed
        friend class Group;  // Allow group to trigger onHandleDestroyed
        friend class Tab;    // Allow tab to trigger onHandleDestroyed
        friend class Box;    // Allow box to trigger onHandleDestroyed
    };

} // namespace mui