// --- core_macros.hpp (New Additions) ---
#pragma once
#include <memory>

namespace mui
{

    // 1. CRTP Base for clean method chaining
    template <typename T>
    class Chainable : public std::enable_shared_from_this<T>
    {
    protected:
        std::shared_ptr<T> self()
        {
            return this->shared_from_this();
        }
    };

// 2. Exception-Safe C-Callback Stub Generator
#define MUI_IMPL_STATELESS_CB(ClassType, UiType, StubName, CbName) \
    void ClassType::StubName(UiType *, void *data)                 \
    {                                                              \
        auto instance = static_cast<ClassType *>(data);            \
        if (instance->CbName)                                      \
        {                                                          \
            try                                                    \
            {                                                      \
                instance->CbName();                                \
            }                                                      \
            catch (...)                                            \
            {                                                      \
                /* Prevents C++ exceptions crashing libui */       \
            }                                                      \
        }                                                          \
    }

} // namespace mui