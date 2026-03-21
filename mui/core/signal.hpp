// core/signal.hpp
#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <mutex>
#include <algorithm>

namespace mui
{
    class Connection
    {
    private:
        // The shared_ptr control block inherently manages the alive state.
        // When all Connection instances are destroyed, the weak_ptr in the Signal expires.
        std::shared_ptr<bool> m_alive;

    public:
        Connection() = default;
        explicit Connection(std::shared_ptr<bool> alive) : m_alive(std::move(alive)) {}
        
        // Explicitly drop the connection
        void disconnect() 
        {
            m_alive.reset(); 
        }

        // Check if still connected (hasn't been manually disconnected)
        bool connected() const 
        {
            return m_alive != nullptr;
        }
    };

    template <typename... Args>
    class Signal
    {
    private:
        struct Slot {
            std::weak_ptr<bool> alive;
            std::function<void(Args...)> cb;
        };
        std::vector<Slot> m_slots;
        mutable std::mutex m_mutex;

    public:
        Signal() = default;
        ~Signal() = default;

        // Non-copyable to prevent accidental shared state issues
        Signal(const Signal&) = delete;
        Signal& operator=(const Signal&) = delete;

        [[nodiscard]] Connection connect(std::function<void(Args...)> cb)
        {
            if (!cb) return Connection();

            std::lock_guard<std::mutex> lock(m_mutex);
            auto alive = std::make_shared<bool>(true);
            m_slots.push_back({alive, std::move(cb)});
            return Connection(alive);
        }

        void operator()(Args... args)
        {
            std::vector<std::function<void(Args...)>> callbacks;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                
                // Cleanup dead connections and collect active ones
                auto it = m_slots.begin();
                while (it != m_slots.end()) {
                    if (it->alive.expired()) {
                        it = m_slots.erase(it);
                    } else {
                        callbacks.push_back(it->cb);
                        ++it;
                    }
                }
            }
            
            // Execute outside the lock to allow re-entrancy and avoid deadlocks
            for (const auto& cb : callbacks) {
                cb(args...);
            }
        }
        
        size_t slot_count() const 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return std::count_if(m_slots.begin(), m_slots.end(), [](const Slot& s) { return !s.alive.expired(); });
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_slots.clear();
        }
    };
} // namespace mui
