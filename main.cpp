#include <mui.hpp>
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
        auto win = Window::create("MUI Control Gallery", 640, 480, true);
        win->setMargined(true);
        win->onClosing([]() {
            App::quit();
            return true;
        });

        // 2. Root Container: Tab
        auto tabs = Tab::create();
        win->setChild(tabs);

        // --- TAB 1: Basic Controls ---
        auto vboxBasics = VBox::create();
        vboxBasics->setPadded(true);

        // Normalized to make_shared to match the refactored class constructors
        auto lblStatus = Label::create("Waiting for action...");
        auto btnClick = Button::create(ICON_FA_FLOPPY_DISK " Click Me");
        auto chkToggle = Checkbox::create("Enable Feature X");

        btnClick->onClick([lblStatus]() {
            lblStatus->setText("Button was clicked!");
        });

        chkToggle->onToggled([lblStatus, chkToggle]() {
            lblStatus->setText(chkToggle->isChecked() ? "Feature X Enabled" : "Feature X Disabled");
        });

        append_all(vboxBasics, {
            {lblStatus},
            {btnClick},
            {chkToggle}
        });

        tabs->append("Basics", vboxBasics);
        tabs->setMargined(0, true);

        // --- TAB 2: Numbers & Progress ---
        auto vboxNumbers = VBox::create();
        vboxNumbers->setPadded(true);

        auto spinBox = Spinbox::create(0, 100);
        auto slider = SliderInt::create(0, 100);
        auto progressBar = ProgressBar::create();

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

        append_all(vboxNumbers, {
            {Label::create("Sync Test:"), true},
            {spinBox, true},
            {slider, true},
            {progressBar, true}
        });

        tabs->append("Numbers", vboxNumbers);
        tabs->setMargined(1, true);

        // --- TAB 3: Text Entries ---
        auto vboxText = VBox::create();
        vboxText->setPadded(true);

        auto entryStandard = Entry::create();
        auto entryPassword = PasswordEntry::create();
        auto entrySearch = SearchEntry::create();
        auto lblMirror = Label::create("Mirror: ");

        entryStandard->onChanged([entryStandard, lblMirror]() {
            lblMirror->setText("Mirror: " + entryStandard->getText());
        });

        append_all(vboxText, {
            {Label::create("Standard Entry:")},
            {entryStandard},
            {Label::create("Password Entry:")},
            {entryPassword, true},
            {Label::create("Search Entry:")},
            {entrySearch},
            {lblMirror}
        });

        tabs->append("Text Entries", vboxText);
        tabs->setMargined(2, true);

        // --- TAB 4: Groups, Layouts & Dialogs ---
        auto vboxDialogs = VBox::create();
        vboxDialogs->setPadded(true);

        auto group = Group::create("System Dialogs");
        group->setMargined(true);

        auto hboxButtons = HBox::create();
        hboxButtons->setPadded(true);

        auto btnInfo = Button::create("Info Msg");
        auto btnError = Button::create("Error Msg");
        auto btnFile = Button::create("Open File");

        btnInfo->setUseContainerWidth(true)->onClick([win]() {
            Dialogs::msgBox(*win, "Information", "This is a standard message box testing the C++ wrapper.");
        });

        btnError->setUseContainerWidth(true)->onClick([win]() {
            Dialogs::msgBoxError(*win, "Critical Error", "Failed to load imaginary resources.");
        });

        btnFile->setUseContainerWidth(true)->onClick([win]() {
            std::string path = Dialogs::openFile(*win);
            if (!path.empty()) {
                Dialogs::msgBox(*win, "File Selected", "Path: " + path);
            }
        });

        append_all(hboxButtons, {
            {btnInfo, true},
            {btnError, true},
            {btnFile, true}
        });

        group->setChild(hboxButtons);
        vboxDialogs->append(group, true);

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