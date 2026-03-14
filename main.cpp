#include "mui_sdl_imgui/mui.hpp"
#include <iostream>
#include <memory>
#include <string>

using namespace mui;

int main()
{
    // try
    // {
        App::init();

        // 1. Main Window
        auto win = std::make_shared<Window>("MUI Control Gallery", 640, 480, true);
        win->setMargined(true);
        win->onClosing([]() {
            App::quit();
            return true;
        });

        // 2. Root Container: Tab
        auto tabs = std::make_shared<Tab>();
        win->setChild(tabs);

        // --- TAB 1: Basic Controls ---
        auto vboxBasics = std::make_shared<VBox>();
        vboxBasics->setPadded(true);

        // Normalized to make_shared to match the refactored class constructors
        auto lblStatus = std::make_shared<Label>("Waiting for action...");
        auto btnClick = std::make_shared<Button>("Click Me");
        auto chkToggle = std::make_shared<Checkbox>("Enable Feature X");

        btnClick->onClick([lblStatus]() {
            lblStatus->setText("Button was clicked!");
        });

        chkToggle->onToggled([lblStatus, chkToggle]() {
            lblStatus->setText(chkToggle->isChecked() ? "Feature X Enabled" : "Feature X Disabled");
        });

        vboxBasics->append(lblStatus, false)
                  ->append(btnClick, false)
                  ->append(chkToggle, false);

        tabs->append("Basics", vboxBasics);
        tabs->setMargined(0, true);

        // --- TAB 2: Numbers & Progress ---
        auto vboxNumbers = std::make_shared<VBox>();
        vboxNumbers->setPadded(true);

        auto spinBox = std::make_shared<Spinbox>(0, 100);
        auto slider = std::make_shared<Slider>(0, 100);
        auto progressBar = std::make_shared<ProgressBar>();

        // Synchronize all three controls
        auto syncFunc = [spinBox, slider, progressBar](int value) {
            spinBox->setValue(value);
            slider->setValue(value);
            progressBar->setValue(value);
        };

        spinBox->onChanged([spinBox, syncFunc]() { syncFunc(spinBox->getValue()); });
        slider->onChanged([slider, syncFunc]() { syncFunc(slider->getValue()); });

        // Initialize to 50
        syncFunc(50);

        vboxNumbers->append(std::make_shared<Label>("Sync Test:"), false)
                   ->append(spinBox, false)
                   ->append(slider, false)
                   ->append(progressBar, false);

        tabs->append("Numbers", vboxNumbers);
        tabs->setMargined(1, true);

        // --- TAB 3: Text Entries ---
        auto vboxText = std::make_shared<VBox>();
        vboxText->setPadded(true);

        auto entryStandard = std::make_shared<Entry>();
        auto entryPassword = std::make_shared<PasswordEntry>();
        auto entrySearch = std::make_shared<SearchEntry>();
        auto lblMirror = std::make_shared<Label>("Mirror: ");

        entryStandard->onChanged([entryStandard, lblMirror]() {
            lblMirror->setText("Mirror: " + entryStandard->getText());
        });

        vboxText->append(std::make_shared<Label>("Standard Entry:"), false)
                ->append(entryStandard, false)
                ->append(std::make_shared<Label>("Password Entry:"), false)
                ->append(entryPassword, false)
                ->append(std::make_shared<Label>("Search Entry:"), false)
                ->append(entrySearch, false)
                ->append(lblMirror, false);

        tabs->append("Text Entries", vboxText);
        tabs->setMargined(2, true);

        // --- TAB 4: Groups, Layouts & Dialogs ---
        auto vboxDialogs = std::make_shared<VBox>();
        vboxDialogs->setPadded(true);

        auto group = std::make_shared<Group>("System Dialogs");
        group->setMargined(true);

        auto hboxButtons = std::make_shared<HBox>();
        hboxButtons->setPadded(true);

        auto btnInfo = std::make_shared<Button>("Info Msg");
        auto btnError = std::make_shared<Button>("Error Msg");
        auto btnFile = std::make_shared<Button>("Open File");

        btnInfo->onClick([win]() {
            Dialogs::msgBox(*win, "Information", "This is a standard message box testing the C++ wrapper.");
        });

        btnError->onClick([win]() {
            Dialogs::msgBoxError(*win, "Critical Error", "Failed to load imaginary resources.");
        });

        btnFile->onClick([win]() {
            std::string path = Dialogs::openFile(*win);
            if (!path.empty()) {
                Dialogs::msgBox(*win, "File Selected", "Path: " + path);
            }
        });

        hboxButtons->append(btnInfo, true)
                   ->append(btnError, true)
                   ->append(btnFile, true);

        group->setChild(hboxButtons);
        vboxDialogs->append(group, false);

        tabs->append("Dialogs & Groups", vboxDialogs);
        tabs->setMargined(3, true);

        // 3. Show and Run
        win->show();
        App::run();
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "Fatal Exception: " << e.what() << std::endl;
    //     return 1;
    // }

    return 0;
}