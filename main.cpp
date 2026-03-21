// main.cpp
#include <mui.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <ctime>
#include <imgui_internal.h>
#include "mui/core/docking.hpp"

// Include the newly created advanced controls
#include "mui/widgets/toggleswitch.hpp"
#include "mui/widgets/rangeslider.hpp"
#include "mui/widgets/iconstack.hpp"
#include "mui/layouts/splitter.hpp"
#include "mui/widgets/table.hpp"
#include "mui/widgets/breadcrumb.hpp"

using namespace mui;

// Forward declarations for UI creation functions
IControlPtr createBasicsTab(const LabelPtr &statusLabel);
IControlPtr createNumbersTab(const LabelPtr &statusLabel);
IControlPtr createTextEntriesTab(const LabelPtr &statusLabel);
IControlPtr createDialogsTab(const WindowPtr &parentWindow, const LabelPtr &statusLabel);
IControlPtr createLayoutsTab(const LabelPtr &statusLabel);
IControlPtr createMoreControlsTab(const LabelPtr &statusLabel);
IControlPtr createAdvancedTab(const LabelPtr &statusLabel);
IControlPtr createThemesTab();
WindowPtr createMainGalleryWindow();
WindowPtr createInspectorWindow();

int main()
{
    App::init();
    App::setTheme(ThemeType::Light);

    // Define a docking layout for our windows to arrange them on startup.
    App::setLayoutBuilder([](mui::DockBuilder &builder)
                          {
            ImGuiID dockspace_id = builder.getID();
            ImGuiDockNode *root_node = ImGui::DockBuilderGetNode(dockspace_id);
            bool is_new_layout = (root_node == nullptr || root_node->IsEmpty());

            // Part 1: Build the layout only if it doesn't exist (e.g., on first run).
            if (is_new_layout)
            {
                // Start with the full dockspace ID. After the split, this ID will refer to the left node.
                ImGuiID center_node_id = dockspace_id;
                ImGuiID right_node_id = builder.splitNode(center_node_id, mui::DockDirection::Right, 0.30f);

                builder.getNode(center_node_id).setHiddenTabBar(true); // Hide the tab bar for the center node
                builder.getNode(right_node_id).setHiddenTabBar(true); // Hide the tab bar for the right node
                // Dock windows. This only needs to be done once.
                builder.dockWindow("MUI Control Gallery", center_node_id);
                builder.dockWindow("Inspector", right_node_id);
            }

            // To create a seamless look without the dockspace's own title bars,
            // we disable the tab bar on the dock nodes. This needs to run every time,
            // as these flags are not saved in the .ini file.
            ImGuiWindow *gallery_window = ImGui::FindWindowByName("MUI Control Gallery");
            if (gallery_window && gallery_window->DockNode)
                builder.getNode(gallery_window->DockNode->ID).setNoTabBar(true);
            ImGuiWindow *inspector_window = ImGui::FindWindowByName("Inspector");
            if (inspector_window && inspector_window->DockNode)
                builder.getNode(inspector_window->DockNode->ID).setNoTabBar(true); });

    App::setMainLoopCallback(
        []()
        {
            mui::Dialogs::processDialogs();
        });
    Control<Button>::setGlobalShadowDefaults(false, ImVec2(0.0f, 0.0f), 4.0f, ImVec4(0.12f, 0.53f, 0.90f, 0.08f), 8.0f);

    // Create and show the main window with the control gallery
    auto mainWin = createMainGalleryWindow();
    mainWin->show();

    // Create and show a second, independent "Inspector" window
    auto inspectorWin = createInspectorWindow();
    inspectorWin->show();

    // Run the application's main loop
    App::run();

    return 0;
}

/**
 * @brief Creates the main window, which contains a tabbed interface
 * to showcase various controls.
 */
WindowPtr createMainGalleryWindow()
{
    auto win = Window::create("MUI Control Gallery", 800, 600);
    win->setMargined(true); // Let the window handle the main padding.
    win->onClosing([]()
                   {
        App::quit();
        return true; });

    // Root container for the window, allows for a status bar
    auto rootVBox = VBox::create();
    rootVBox->setPadded(false); // Window margin is enough.
    win->setChild(rootVBox);

    // Tabs for the main content area
    auto tabs = Tab::create();
    rootVBox->append(tabs, true); // Stretchy, fills available space

    // Status bar at the bottom, with a separator for visual clarity.
    auto lblStatus = Label::create("Waiting for action...");
    rootVBox->append(Separator::create()->setType(SeparatorType::Native), false);
    rootVBox->append(lblStatus, false);

    // Now, create and add the content for each tab
    tabs->append("Basics", createBasicsTab(lblStatus));
    tabs->append("Numbers", createNumbersTab(lblStatus));
    tabs->append("Text", createTextEntriesTab(lblStatus));
    tabs->append("Dialogs", createDialogsTab(win, lblStatus));
    tabs->append("Layouts", createLayoutsTab(lblStatus));
    tabs->append("Advanced", createAdvancedTab(lblStatus)); // Added Advanced Tab for new widgets
    tabs->append("More", createMoreControlsTab(lblStatus));
    tabs->append("Themes", createThemesTab());

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
    root->setPadded(false); // Window margin is enough.
    win->setChild(root);

    // --- Card Example ---
    auto card = Card::create();
    auto cardContent = VBox::create();
    cardContent->setPadded(true);
    card->setChild(cardContent);
    card->setFillHeight(false);
    card->defaultShadow()          // setShadow(true, {0.0f, 0.0f}, 18.0f, {0.12f, 0.53f, 0.90f, 0.1f}, 8.0f)
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

IControlPtr createBasicsTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();

    // IconStack demonstration (Horizontal Toolbar)
    auto iconStack = IconStack::create()
                         ->add(ICON_FA_PLAY, [lblStatus]()
                               { lblStatus->setText("Play clicked"); }, "Play Action")
                         ->add(ICON_FA_PAUSE, [lblStatus]()
                               { lblStatus->setText("Pause clicked"); }, "Pause Action")
                         ->add(ICON_FA_STOP, [lblStatus]()
                               { lblStatus->setText("Stop clicked"); }, "Stop Action");

    auto btnClick = Button::create(ICON_FA_FLOPPY_DISK " Click Me");
    btnClick->onClick([lblStatus]()
                      {
                          auto currentTime = std::time(nullptr);
                          char timeStr[100];
                          std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&currentTime));
                          lblStatus->setText(std::string("Button clicked at ") + timeStr); })
        ->defaultShadow();

    auto chkToggle = Checkbox::create("Standard Checkbox");
    chkToggle->onToggled([lblStatus, chkToggle]()
                         { lblStatus->setText(chkToggle->isChecked() ? "Checkbox Enabled" : "Checkbox Disabled"); });
    chkToggle->setScale(1.2f); // Make the checkbox larger for better visibility

    // Modern ToggleSwitch demonstration
    auto modernToggle = ToggleSwitch::create("Modern iOS-Style Toggle");
    modernToggle->onToggled([lblStatus](bool state)
                            { lblStatus->setText(state ? "Modern Toggle is ON" : "Modern Toggle is OFF"); });
    modernToggle->setScale(0.6f); // Make the toggle larger for better visibility

    auto btnFindNode = Button::create(ICON_FA_MAGNIFYING_GLASS " Find Inspector Dock Node");
    btnFindNode->onClick([lblStatus]()
                         {
        const char* window_title = "Inspector";
        // Use ImGui's internal API to find the window by its title.
        ImGuiWindow* window = ImGui::FindWindowByName(window_title);

        // If the window is found and it is docked, get its DockNode.
        if (window && window->DockNode) {
            ImGuiDockNode* node = window->DockNode;
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Inspector is in DockNode ID: 0x%08X. Tabs in this node: %d", node->ID, node->TabBar ? node->TabBar->Tabs.Size : 0);
            lblStatus->setText(buffer);
        } else {
            lblStatus->setText("Could not find a docked 'Inspector' window.");
        } });

    append_all(vbox, {{Label::create("Media Controls (IconStack):")},
                      {iconStack},
                      {Separator::create()->setType(SeparatorType::Native)},
                      {btnClick},
                      {chkToggle},
                      {modernToggle},
                      {Separator::create()->setType(SeparatorType::Native)},
                      {btnFindNode}});
    return vbox;
}

IControlPtr createNumbersTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();

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
                       { syncFunc(spinBox->getValue()); })
        ->defaultShadow();
    slider->onChanged([slider, syncFunc]()
                      { syncFunc(slider->getValue()); })
        ->defaultShadow();

    syncFunc(50); // Initialize to 50

    // Range Slider Demonstration
    auto rangeSlider = RangeSlider::create(0.0f, 100.0f);
    rangeSlider->setRange(20.0f, 80.0f);
    rangeSlider->setSpanAvailWidth(true);
    rangeSlider->onChanged([lblStatus](float vMin, float vMax)
                           {
        char buf[128];
        snprintf(buf, sizeof(buf), "Range changed: %.1f to %.1f", vMin, vMax);
        lblStatus->setText(buf); });

    append_all(vbox, {{Label::create("Sync Test:"), false},
                      {spinBox, false},
                      {slider, false},
                      {progressBar, false},
                      {Separator::create()->setType(SeparatorType::Native)},
                      {Label::create("Dual Handle Range Slider:")},
                      {rangeSlider, false}});
    return vbox;
}

IControlPtr createTextEntriesTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();

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

IControlPtr createDialogsTab(const WindowPtr &win, const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();

    auto group = Group::create("System Dialogs");
    group->setMargined(true);

    auto hboxButtons = HBox::create();
    hboxButtons->setPadded(true);

    auto btnInfo = Button::create(ICON_FA_CIRCLE_INFO " Info");
    auto btnWarning = Button::create(ICON_FA_TRIANGLE_EXCLAMATION " Warning");
    auto btnError = Button::create(ICON_FA_CIRCLE_XMARK " Error");
    auto btnConfirm = Button::create(ICON_FA_CIRCLE_CHECK " Confirm");
    auto btnQuestion = Button::create(ICON_FA_CIRCLE_QUESTION " Question");
    auto btnCustom = Button::create(ICON_FA_GEARS " Custom");
    auto btnFile = Button::create(ICON_FA_FOLDER_OPEN " Open File");

    btnInfo->onClick(
               [win, lblStatus]()
               {
                   Dialogs::msgBoxInfo("Information", "This is an informational message box, which is the default type.");
                   lblStatus->setText("Info dialog shown.");
               })
        ->defaultShadow();

    btnWarning->onClick(
                  [win, lblStatus]()
                  {
                      Dialogs::msgBoxWarning("Warning", "This is a warning message. Something might be wrong, so you should pay attention.");
                      lblStatus->setText("Warning dialog shown.");
                  })
        ->defaultShadow();

    btnError->onClick(
        [win, lblStatus]()
        {
            Dialogs::msgBoxError("Critical Error", "Failed to load imaginary resources.");
            lblStatus->setText("Error dialog shown.");
        });

    btnConfirm->onClick(
                  [win, lblStatus]()
                  {
                      Dialogs::msgBoxConfirm("Confirmation", "This action is permanent. Are you sure you want to proceed?", [lblStatus]()
                                             { lblStatus->setText("Confirmed!"); }, [lblStatus]()
                                             { lblStatus->setText("Cancelled."); });
                  })
        ->defaultShadow();

    btnQuestion->onClick(
                   [win, lblStatus]()
                   {
                       Dialogs::msgBoxQuestion("Unsaved Work", "Do you want to save your changes before closing?", [lblStatus]()
                                               { lblStatus->setText("Answered: Yes"); }, [lblStatus]()
                                               { lblStatus->setText("Answered: No"); });
                   })
        ->defaultShadow();

    btnCustom->onClick(
        [lblStatus]()
        {
            Dialogs::msgBoxCustom("File Operation", "The destination file already exists.", MessageBoxType::Warning,
                                  {{"Overwrite", [lblStatus]()
                                    { lblStatus->setText("File Overwritten."); }},
                                   {"Skip", [lblStatus]()
                                    { lblStatus->setText("File Skipped."); }},
                                   {"Cancel", [lblStatus]()
                                    { lblStatus->setText("Operation Cancelled."); }}});
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
                             {btnWarning, true},
                             {btnError, true},
                             {btnConfirm, true},
                             {btnQuestion, true},
                             {btnCustom, true},
                             {btnFile, true}});

    group->setChild(hboxButtons);
    vbox->append(group, true);
    return vbox;
}

IControlPtr createLayoutsTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();

    auto gridGroup = Group::create("Grid Layout");
    gridGroup->setMargined(true);
    vbox->append(gridGroup, false); // Let the group have its natural height

    auto grid = Grid::create();
    grid->setColumnWeight(0, 0.3f); // Label column
    grid->setColumnWeight(1, 0.7f); // Entry column
    gridGroup->setChild(grid);

    grid->append(Label::create("First Name:"), 0, 0);
    auto firstNameEntry = Entry::create("John");
    grid->append(firstNameEntry, 0, 1);

    grid->append(Label::create("Last Name:"), 1, 0);
    auto lastNameEntry = Entry::create("Doe");
    grid->append(lastNameEntry, 1, 1);

    grid->append(Label::create("Address:"), 2, 0);
    auto addressEntry = Entry::create("123 Main St, Anytown");
    grid->append(addressEntry, 2, 1);

    auto submitButton = Button::create("Submit");
    submitButton->onClick([lblStatus, firstNameEntry, lastNameEntry, addressEntry]()
                          {
        std::string text = "Submitted: " + firstNameEntry->getText() + " " + lastNameEntry->getText() + " from " + addressEntry->getText();
        lblStatus->setText(text); });
    grid->append(Label::create(""), 3, 0); // Empty cell to push button to the right column
    grid->append(submitButton, 3, 1);

    vbox->append(Separator::create()->setType(SeparatorType::Native));

    auto flowGroup = Group::create("Flow Layout (Left)");
    flowGroup->setMargined(true);
    vbox->append(flowGroup, false);

    auto flowBox = FlowBox::create()->setAlign(FlowBox::Align::Left);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Button " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " is a bit longer";
        }
        flowBox->append(Button::create(label)->onClick([lblStatus, label]()
                                                       { lblStatus->setText(label + " clicked"); }));
    }
    flowGroup->setChild(flowBox);

    auto flowGroupCenter = Group::create("Flow Layout (Center)");
    flowGroupCenter->setMargined(true);
    vbox->append(flowGroupCenter, false);
    auto flowBoxCenter = FlowBox::create()->setAlign(FlowBox::Align::Center);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Button " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " is a bit longer";
        }
        flowBoxCenter->append(Button::create(label)->onClick([lblStatus, label]()
                                                             { lblStatus->setText(label + " clicked"); }));
    }
    flowGroupCenter->setChild(flowBoxCenter);

    auto flowGroupRight = Group::create("Flow Layout (Right)");
    flowGroupRight->setMargined(true);
    vbox->append(flowGroupRight, false);
    auto flowBoxRight = FlowBox::create()->setAlign(FlowBox::Align::Right);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Button " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " is a bit longer";
        }
        flowBoxRight->append(Button::create(label)->onClick([lblStatus, label]()
                                                            { lblStatus->setText(label + " clicked"); }));
    }
    flowGroupRight->setChild(flowBoxRight);

    auto flowGroupJustify = Group::create("Flow Layout (Justify)");
    flowGroupJustify->setMargined(true);
    vbox->append(flowGroupJustify, false);
    auto flowBoxJustify = FlowBox::create()->setAlign(FlowBox::Align::Justify);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Btn " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " long";
        }
        flowBoxJustify->append(Button::create(label)->onClick([lblStatus, label]()
                                                              { lblStatus->setText(label + " clicked"); }));
    }
    flowGroupJustify->setChild(flowBoxJustify);

    auto flowGroupFill = Group::create("Flow Layout (Fill)");
    flowGroupFill->setMargined(true);
    vbox->append(flowGroupFill, false);
    auto flowBoxFill = FlowBox::create()->setAlign(FlowBox::Align::Fill);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Btn " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " long";
        }
        flowBoxFill->append(Button::create(label)->onClick([lblStatus, label]()
                                                           { lblStatus->setText(label + " clicked"); }));
    }
    flowGroupFill->setChild(flowBoxFill);

    return vbox;
}

IControlPtr createAdvancedTab(const LabelPtr &lblStatus)
{
    // This tab demonstrates a complex File Explorer UI built with the new widgets.
    auto vbox = VBox::create();
    vbox->setPadded(false); // Remove padding so the splitter takes full space naturally

    // --- Toolbar Area ---
    auto toolbarArea = VBox::create();
    toolbarArea->setPadded(true);

    auto toolbarHBox = HBox::create();
    toolbarHBox->setPadded(true);

    // Breadcrumb Navigator
    auto breadcrumb = BreadcrumbBar::create("C:/Users/Public/Documents");
    breadcrumb->onPathNavigated([lblStatus](const std::string &path)
                                { lblStatus->setText("Navigated to: " + path); });

    // Navigation Buttons (IconStack)
    auto navButtons = IconStack::create()
                          ->add(ICON_FA_ARROW_LEFT, [lblStatus]()
                                { lblStatus->setText("Back clicked"); }, "Go Back")
                          ->add(ICON_FA_ARROW_RIGHT, [lblStatus]()
                                { lblStatus->setText("Forward clicked"); }, "Go Forward")
                          ->add(ICON_FA_ARROW_UP, [lblStatus]()
                                { lblStatus->setText("Up clicked"); }, "Up to Parent Folder");

    toolbarHBox->append(navButtons, false);
    toolbarHBox->append(breadcrumb, true); // Stretches to fill width

    toolbarArea->append(toolbarHBox, false);
    toolbarArea->append(Separator::create()->setType(SeparatorType::Native), false);
    vbox->append(toolbarArea, false);

    // --- Splitter Area ---
    auto splitter = SplitterView::create(SplitterOrientation::Horizontal);
    splitter->setSplitRatio(0.25f);

    // Left Panel: Directory Tree
    auto leftPanel = VBox::create();
    leftPanel->setPadded(true);

    auto treeRoot = TreeNode::create(ICON_FA_DESKTOP " This PC")->setDefaultOpen(true);
    treeRoot->append(Label::create(ICON_FA_HARD_DRIVE " Local Disk (C:)"));
    auto driveD = TreeNode::create(ICON_FA_HARD_DRIVE " Data (D:)");
    driveD->append(Label::create(ICON_FA_FOLDER " Backups"));
    driveD->append(Label::create(ICON_FA_FOLDER " Projects"));
    treeRoot->append(driveD);

    leftPanel->append(treeRoot, true);
    splitter->setPanel1(leftPanel);

    // Right Panel: Data Table
    auto table = Table::create();
    table->addColumn("Name", 3.0f)->addColumn("Date Modified", 2.0f)->addColumn("Size", 1.0f);
    table->setSortable(true);
    table->setSpanAvailWidth(true);

    // Add some mock file data
    table->addRow({Label::create(ICON_FA_FOLDER " Photos"), Label::create("10/24/2023"), Label::create("--")});
    table->addRow({Label::create(ICON_FA_FOLDER " Videos"), Label::create("10/12/2023"), Label::create("--")});
    table->addRow({Label::create(ICON_FA_FILE_WORD " Q4_Report.docx"), Label::create("10/25/2023"), Label::create("2,450 KB")});
    table->addRow({Label::create(ICON_FA_FILE_EXCEL " Budget_2024.xlsx"), Label::create("10/26/2023"), Label::create("124 KB")});
    table->addRow({Label::create(ICON_FA_FILE_PDF " Invoice_992.pdf"), Label::create("10/27/2023"), Label::create("89 KB")});

    table->onRowSelected([lblStatus](int row)
                         { lblStatus->setText("Selected table row: " + std::to_string(row)); });

    table->onSortRequested([lblStatus](int colIndex, bool ascending)
                           {
        std::string dir = ascending ? "Ascending" : "Descending";
        lblStatus->setText("Sort requested on column " + std::to_string(colIndex) + " (" + dir + ")"); });

    auto rightPanel = VBox::create();
    rightPanel->append(table, true);
    splitter->setPanel2(rightPanel);

    vbox->append(splitter, true); // Stretchy to take remaining height below toolbar
    return vbox;
}

IControlPtr createMoreControlsTab(const LabelPtr &lblStatus)
{
    auto vbox = VBox::create();

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

IControlPtr createThemesTab()
{
    auto vbox = VBox::create();

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