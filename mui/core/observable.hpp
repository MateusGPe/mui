// core/observable.hpp
#pragma once
#include <mutex>
#include "signal.hpp"

namespace mui
{
    template <typename T>
    class Observable
    {
    private:
        T m_value;
        mutable std::mutex m_mutex;

    public:
        mui::Signal<const T&> onValueChanged;

        Observable() = default;
        explicit Observable(const T& initialValue) : m_value(initialValue) {}

        // Non-copyable, thread-safe access only
        Observable(const Observable&) = delete;
        Observable& operator=(const Observable&) = delete;

        void set(const T& new_val)
        {
            bool changed = false;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_value != new_val)
                {
                    m_value = new_val;
                    changed = true;
                }
            }

            // Emit the signal outside the lock to prevent deadlocks 
            // if listeners try to get/set the value or trigger UI updates.
            if (changed)
            {
                onValueChanged(new_val);
            }
        }

        T get() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_value;
        }

        // Convenience operators
        Observable& operator=(const T& new_val)
        {
            set(new_val);
            return *this;
        }

        operator T() const
        {
            return get();
        }
    };
} // namespace mui
