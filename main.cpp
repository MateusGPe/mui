#include "mui/mui.hpp"
#include <iostream>

std::shared_ptr<mui::Window> buildNiflInterface()
{
    // 1. Instantiate interactive components
    auto promptEntry = mui::Entry::create();
    auto negPromptEntry = mui::Entry::create();
    auto generateBtn = mui::Button::create("Generate");
    auto progressBar = mui::ProgressBar::create();

    // 2. Wire behaviors (using weak pointers to avoid memory leaks)
    std::weak_ptr<mui::Entry> weakPrompt = promptEntry;
    std::weak_ptr<mui::ProgressBar> weakProgress = progressBar;

    generateBtn->onClick([weakPrompt, weakProgress]()
                         {
    if (auto prompt = weakPrompt.lock()) {
      std::cout << "Dispatching to stable-diffusion.cpp: " << prompt->getText()
                << std::endl;
    }
    if (auto progress = weakProgress.lock()) {
      progress->setValue(10); // Simulated progress
    } });

    // 3. Assemble the UI declaratively
    auto mainWindow =
        mui::Window::create("Nifl - Stable Diffusion", 800, 600)
            ->setMargined(true)
            ->setChild(
                mui::VBox::create()
                    ->setPadded(true)
                    ->append(
                        mui::Group::create("Parameters")
                            ->setMargined(true)
                            ->setChild(mui::VBox::create()
                                           ->setPadded(true)
                                           ->append(mui::Label::create("Prompt"))
                                           ->append(promptEntry)
                                           ->append(mui::Label::create("Negative Prompt"))
                                           ->append(negPromptEntry)))
                    ->append(generateBtn)
                    ->append(progressBar));

    mainWindow->onClosing(
        []()
        {
            mui::App::quit();
            return true;
        });

    mainWindow->show();
    return mainWindow;
}

int main()
{
    mui::App::init();
    auto mainWin = buildNiflInterface();
    mui::App::run();
    return 0;
}