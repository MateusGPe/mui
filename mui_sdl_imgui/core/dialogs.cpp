#include "dialogs.hpp"
#include "control.hpp"
#include <SDL3/SDL.h>

namespace mui
{
    void Dialogs::msgBox(Control &parent, const std::string &title, const std::string &description)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), description.c_str(), nullptr);
    }

    void Dialogs::msgBoxError(Control &parent, const std::string &title, const std::string &description)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), description.c_str(), nullptr);
    }

    std::string Dialogs::openFile(Control &parent)
    {
        return ""; 
    }
} // namespace mui
