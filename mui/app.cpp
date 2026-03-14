#include "app.hpp"
#include <memory>
#include <stdexcept>
#include <ui.h>

namespace mui
{

    std::thread::id App::mainThreadId;

    void App::init()
    {
        mainThreadId = std::this_thread::get_id();
        uiInitOptions options = {};
        const char *err = uiInit(&options);
        if (err != nullptr)
        {
            // Immediate copy and free, avoiding incomplete type errors with unique_ptr
            std::string errorStr(err);
            uiFreeInitError(err);
            throw std::runtime_error("Failed to initialize libui-ng: " + errorStr);
        }
    }

    void App::run() { uiMain(); }

    void App::quit() { uiQuit(); }

    void App::assertMainThread()
    {
        if (std::this_thread::get_id() != mainThreadId)
        {
            throw std::runtime_error(
                "MUI Error: UI interaction outside of the main thread.");
        }
    }

    void App::queueMain(std::function<void()> callback)
    {
        auto *cb = new std::function<void()>(std::move(callback));
        uiQueueMain(
            [](void *data)
            {
                // Take ownership back from the C-API to guarantee cleanup
                std::unique_ptr<std::function<void()>> func(
                    static_cast<std::function<void()> *>(data));
                (*func)();
            },
            cb);
    }

} // namespace mui