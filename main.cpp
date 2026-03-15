#include <mui.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
using namespace mui;

// Forward declarations for UI creation functions
ControlPtr createBasicsTab(const LabelPtr &statusLabel);
ControlPtr createNumbersTab(const LabelPtr &statusLabel);
ControlPtr createTextEntriesTab(const LabelPtr &statusLabel);
ControlPtr createDialogsTab(const WindowPtr &parentWindow, const LabelPtr &statusLabel);
ControlPtr createLayoutsTab(const LabelPtr &statusLabel);
ControlPtr createMoreControlsTab(const LabelPtr &statusLabel);
ControlPtr createThemesTab();
WindowPtr createMainGalleryWindow();
WindowPtr createInspectorWindow();

int main()
{
    try
    {
        App::init();
        App::setTheme(ThemeType::Light);
        Control::setGlobalShadowDefaults(false, {0.0f, 0.0f}, 4.0f, {0.12f, 0.53f, 0.90f, 0.08f}, 8.0f);

        // Create and show the main window with the control gallery
        auto mainWin = createMainGalleryWindow();
        mainWin->show();

        // Create and show a second, independent "Inspector" window
        auto inspectorWin = createInspectorWindow();
        inspectorWin->show();

        // Run the application's main loop
        App::run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

/**
 * @brief Creates the main window, which contains a tabbed interface
 * to showcase various controls.
 */
WindowPtr createMainGalleryWindow()
{
    auto win = Window::create("MUI Control Gallery", 800, 600);
    win->setMargined(false); // Use a VBox for root padding/margin
    win->onClosing([]()
                   {
        App::quit();
        return true; });

    // Root container for the window, allows for a status bar
    auto rootVBox = VBox::create();
    win->setChild(rootVBox);

    // Tabs for the main content area
    auto tabs = Tab::create();
    rootVBox->append(tabs, true); // Stretchy, fills available space

    // Status bar at the bottom
    auto lblStatus = Label::create("Waiting for action...");
    auto statusBar = HBox::create(); // Use HBox for padding
    statusBar->setPadded(true);
    statusBar->append(lblStatus);
    rootVBox->append(statusBar, false); // Not stretchy

    // Now, create and add the content for each tab
    tabs->append("Basics", createBasicsTab(lblStatus));
    tabs->setMargined(0, true);

    tabs->append("Numbers", createNumbersTab(lblStatus));
    tabs->setMargined(1, true);

    tabs->append("Text", createTextEntriesTab(lblStatus));
    tabs->setMargined(2, true);

    tabs->append("Dialogs", createDialogsTab(win, lblStatus));
    tabs->setMargined(3, true);

    tabs->append("Layouts", createLayoutsTab(lblStatus));
    tabs->setMargined(4, true);

    tabs->append("More", createMoreControlsTab(lblStatus));
    tabs->setMargined(5, true);

    tabs->append("Themes", createThemesTab());
    tabs->setMargined(6, true);

    return win;
}

/**
 * @brief Creates a secondary window to demonstrate Cards, TreeNodes,
 * and multi-window capabilities.
 */
WindowPtr createInspectorWindow()
{
    auto win = Window::create("Inspector", 300, 400);
    win->setMargined(true);

    auto root = VBox::create();
    root->setPadded(true);
    win->setChild(root);

    // --- Card Example ---
    auto card = Card::create();
    auto cardContent = VBox::create();
    cardContent->setPadded(true);
    card->setChild(cardContent);
    card->setFillHeight(false);
    card->defaultShadow() //setShadow(true, {0.0f, 0.0f}, 18.0f, {0.12f, 0.53f, 0.90f, 0.1f}, 8.0f)
        ->setSpanAvailWidth(true); // Card will fill the width of the parent

    append_all(cardContent, {{Label::create("This is a Card")},
                             {Separator::create()->setType(SeparatorType::Custom)},
                             {Label::create("This card is set to fill available width. The text below is wrapped to fit.")->setFormat(LabelFormat::Wrapped)}});
    root->append(card, false); // Card has natural height, not stretchy

    // --- TreeNode Example ---
    root->append(Separator::create()->setType(SeparatorType::Native));
    auto tree = TreeNode::create(ICON_FA_FOLDER " Project Files")
                    ->setSpanAvailWidth(true);
    tree->append(Label::create(ICON_FA_FILE_CODE " main.cpp")
                     ->setSpanAvailWidth(true));
    auto subTree = TreeNode::create(ICON_FA_FOLDER " core")
                       ->setSpanAvailWidth(true);
    subTree->append(Button::create(ICON_FA_FILE_CODE " app.cpp")
                        ->setSpanAvailWidth(true));
    subTree->append(Label::create(ICON_FA_FILE_CODE " window.cpp")
                        ->setSpanAvailWidth(true));
    tree->append(subTree);
    tree->append(Label::create(ICON_FA_FILE_IMAGE " icon.png")
                     ->setSpanAvailWidth(true));
    root->append(tree, true);

    return win;
}

// --- TAB CREATION FUNCTIONS ---

ControlPtr createBasicsTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();
    vbox->setPadded(true);

    auto btnClick = Button::create(ICON_FA_FLOPPY_DISK " Click Me");
    btnClick->onClick([lblStatus]()
                      { lblStatus->setText("Button was clicked!"); })->defaultShadow();

    auto chkToggle = Checkbox::create("Enable Feature X");
    chkToggle->onToggled([lblStatus, chkToggle]()
                         { lblStatus->setText(chkToggle->isChecked() ? "Feature X Enabled" : "Feature X Disabled"); });

    append_all(vbox, {{btnClick},
                      {chkToggle}});
    return vbox;
}

ControlPtr createNumbersTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();
    vbox->setPadded(true);

    auto spinBox = Spinbox::create(0, 100);
    auto slider = SliderInt::create(0, 100);
    auto progressBar = ProgressBar::create();

    // Synchronize all three controls
    auto syncFunc = [lblStatus, spinBox, slider, progressBar](int value)
    {
        spinBox->setValue(value);
        slider->setValue(value);
        progressBar->setValue(static_cast<float>(value) / 100.0f);
        lblStatus->setText("Numeric controls synced to: " + std::to_string(value));
    };

    spinBox->onChanged([spinBox, syncFunc]()
                       { syncFunc(spinBox->getValue()); })->defaultShadow();
    slider->onChanged([slider, syncFunc]()
                      { syncFunc(slider->getValue()); })->defaultShadow();

    syncFunc(50); // Initialize to 50

    append_all(vbox, {{Label::create("Sync Test:"), false},
                      {spinBox, true},
                      {slider, true},
                      {progressBar, true}});
    return vbox;
}

ControlPtr createTextEntriesTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();
    vbox->setPadded(true);

    auto entryStandard = Entry::create();
    entryStandard->setHint("Type here...");
    auto entryPassword = PasswordEntry::create();
    entryPassword->setHint("Secret...");
    auto entrySearch = SearchEntry::create();
    entrySearch->setHint("Search...");
    auto lblMirror = Label::create("Mirror: ");

    entryStandard->onChanged([entryStandard, lblMirror]()
                             { lblMirror->setText("Mirror: " + entryStandard->getText()); });
    entrySearch->onChanged([entrySearch, lblStatus]()
                           {
        if (!entrySearch->getText().empty()) {
            lblStatus->setText("Searching for: " + entrySearch->getText());
        } });

    append_all(vbox, {{Label::create("Standard Entry:")},
                      {entryStandard},
                      {Label::create("Password Entry:")},
                      {entryPassword, true},
                      {Label::create("Search Entry:")},
                      {entrySearch},
                      {lblMirror}});
    return vbox;
}

ControlPtr createDialogsTab(const WindowPtr &win, const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();
    vbox->setPadded(true);

    auto group = Group::create("System Dialogs");
    group->setMargined(true);

    auto hboxButtons = HBox::create();
    hboxButtons->setPadded(true);

    auto btnInfo = Button::create("Info Msg");
    auto btnError = Button::create("Error Msg");
    auto btnFile = Button::create("Open File");

    btnInfo->onClick(
        [win, lblStatus]()
        {
            Dialogs::msgBox("Information", "This is a standard message box.");
            lblStatus->setText("Info dialog shown.");
        })->defaultShadow();

    btnError->onClick(
        [win, lblStatus]()
        {
            Dialogs::msgBoxError("Critical Error", "Failed to load imaginary resources.");
            lblStatus->setText("Error dialog shown.");
        });

    btnFile->onClick(
        [win, lblStatus]()
        {
            Dialogs::openFile("Open File", "All Files {*.*}", [lblStatus](const std::string &path)
                              {
                    Dialogs::msgBox("File Selected", "Path: " + path);
                    lblStatus->setText("File selected: " + path); }, [lblStatus]()
                              { lblStatus->setText("File open dialog was cancelled."); });
        });

    append_all(hboxButtons, {{btnInfo, true},
                             {btnError, true},
                             {btnFile, true}});

    group->setChild(hboxButtons);
    vbox->append(group, true);
    return vbox;
}

ControlPtr createLayoutsTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();
    vbox->setPadded(true);

    auto gridGroup = Group::create("Grid Layout");
    gridGroup->setMargined(true);
    vbox->append(gridGroup, true);

    auto grid = Grid::create();
    gridGroup->setChild(grid);

    grid->append(Label::create("First Name:"), 0, 0);
    auto firstNameEntry = Entry::create("John");
    grid->append(firstNameEntry, 0, 1, 2);

    grid->append(Label::create("Last Name:"), 1, 0);
    auto lastNameEntry = Entry::create("Doe");
    grid->append(lastNameEntry, 1, 2, 1);

    grid->append(Label::create("Address:"), 2, 0);
    auto addressEntry = Entry::create("123 Main St, Anytown");
    grid->append(addressEntry, 2, 1, 2); // Span 2 columns

    auto submitButton = Button::create("Submit");
    submitButton->onClick([lblStatus, firstNameEntry, lastNameEntry, addressEntry]()
                          {
        std::string text = "Submitted: " + firstNameEntry->getText() + " " + lastNameEntry->getText() + " from " + addressEntry->getText();
        lblStatus->setText(text); });
    grid->append(submitButton, 3, 2); // Align to the right

    return vbox;
}

ControlPtr createMoreControlsTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();
    vbox->setPadded(true);

    // ComboBox
    vbox->append(Label::create("ComboBox:"), false);
    auto combo = ComboBox::create();
    combo->append("Apple");
    combo->append("Banana");
    combo->append("Cherry");
    combo->setSelectedIndex(1);
    combo->onChanged([combo, lblStatus]()
                     { lblStatus->setText("Selected fruit: " + combo->getText()); });
    vbox->append(combo, false);
    vbox->append(Separator::create());

    // Radio Buttons
    vbox->append(Label::create("Radio Buttons:"), false);
    auto radioHBox = HBox::create();
    radioHBox->setPadded(true);
    auto radio1 = RadioButton::create("AM");
    auto radio2 = RadioButton::create("FM");
    auto radio3 = RadioButton::create("SW");
    RadioButton::group({radio1, radio2, radio3});
    radio2->setChecked(true);
    radio1->onToggled([lblStatus](bool checked)
                      { if(checked) lblStatus->setText("Radio set to AM"); });
    radio2->onToggled([lblStatus](bool checked)
                      { if(checked) lblStatus->setText("Radio set to FM"); });
    radio3->onToggled([lblStatus](bool checked)
                      { if(checked) lblStatus->setText("Radio set to SW"); });
    append_all(radioHBox, {{radio1, true}, {radio2, true}, {radio3, true}});
    vbox->append(radioHBox, false);
    vbox->append(Separator::create());

    // ColorEdit
    vbox->append(Label::create("Color Picker:"), false);
    auto colorEdit = ColorEdit::create(0.2f, 0.8f, 0.4f, 1.0f);
    colorEdit->onChanged([lblStatus, colorEdit]()
                         {
        auto c = colorEdit->getColor();
        char buffer[100];
        snprintf(buffer, 100, "Color changed to: (%.2f, %.2f, %.2f, %.2f)", c[0], c[1], c[2], c[3]);
        lblStatus->setText(buffer); });
    vbox->append(colorEdit, false);
    vbox->append(Separator::create());

    // Float Slider
    vbox->append(Label::create("Float Slider:"), false);
    auto sliderFloat = SliderFloat::create(0.0f, 1.0f);
    sliderFloat->setValue(0.75f);
    sliderFloat->onChanged([lblStatus, sliderFloat]()
                           { lblStatus->setText("Float slider value: " + std::to_string(sliderFloat->getValue())); });
    vbox->append(sliderFloat, false);
    vbox->append(Separator::create());

    // --- Custom Separators ---
    vbox->append(Label::create("Custom Separators:"), false);
    auto hboxSeparators = HBox::create();
    hboxSeparators->setPadded(true);

    hboxSeparators->append(Label::create("Left"));

    // Vertical Separator
    hboxSeparators->append(Separator::create()
                               ->setOrientation(SeparatorOrientation::Vertical)
                               ->setThickness(4.0f)
                               ->setColor({0.8f, 0.2f, 0.2f, 1.0f}));

    hboxSeparators->append(Label::create("Right"));
    vbox->append(hboxSeparators, false);

    // Thick Horizontal Separator as a rectangle
    vbox->append(Separator::create()->setThickness(5.0f)->setAsRect(true));
    vbox->append(Separator::create()->setType(SeparatorType::Native));

    // Text Separator
    vbox->append(Separator::create()
                     ->setText("Text Separator"));

    // Native Separator
    vbox->append(Label::create("Native Separator below:"));
    vbox->append(Separator::create()->setType(SeparatorType::Native));

    return vbox;
}

ControlPtr createThemesTab()
{
    auto vbox = VBox::create();
    vbox->setPadded(true);

    vbox->append(Label::create("Switch application theme:"));

    auto hbox = HBox::create();
    hbox->setPadded(true);

    auto btnLight = Button::create("Light Theme");
    btnLight->onClick([]()
                      { App::setTheme(ThemeType::Light); });

    auto btnDark = Button::create("Dark Theme");
    btnDark->onClick([]()
                     { App::setTheme(ThemeType::Dark); });

    append_all(hbox, {{btnLight, true},
                      {btnDark, true}});

    vbox->append(hbox, true);
    return vbox;
}