#pragma once
#include <memory>
#include <stdexcept>

namespace mui
{
    class Control;
    using ControlPtr = std::shared_ptr<Control>;

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
