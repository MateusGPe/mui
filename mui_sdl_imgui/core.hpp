#pragma once
#include <memory>

namespace mui
{
    template <typename Derived>
    class Chainable
    {
    public:
        std::shared_ptr<Derived> self()
        {
            // static_cast down to Derived*, invoke Control::shared_from_this(), 
            // then static_pointer_cast back to shared_ptr<Derived>.
            return std::static_pointer_cast<Derived>(
                static_cast<Derived*>(this)->shared_from_this()
            );
        }
    };
} // namespace mui