#pragma once
#include <functional>
#include <thread>
#include <vector>
#include <string>

namespace mui
{
    class Window;
    class App
    {
    public:
        static std::thread::id mainThreadId;
        static std::vector<Window*> activeWindows;

        static void init();
        static void run();
        static void quit();
        static void assertMainThread();
        static void queueMain(std::function<void()> callback);
    };
} // namespace mui
