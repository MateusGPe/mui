#pragma once
#include <string>

namespace mui
{
    class Control;

    class Dialogs
    {
    public:
        static void msgBox(Control &parent, const std::string &title, const std::string &description);
        static void msgBoxError(Control &parent, const std::string &title, const std::string &description);
        static std::string openFile(Control &parent);
    };
} // namespace mui
