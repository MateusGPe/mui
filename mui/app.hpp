#pragma once
#include <functional>
#include <string>
#include <thread>

namespace mui
{

  // --- Core Application Management ---
  class App
  {
  private:
    static std::thread::id mainThreadId;

  public:
    static void init();
    static void run();
    static void quit();
    static void queueMain(std::function<void()> callback);

    // Ensures the current thread is the thread that called init()
    static void assertMainThread();
  };

} // namespace mui