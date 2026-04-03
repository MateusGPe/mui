// main.cpp
#include <mui.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <random>
#include <algorithm>
#include <ctime>
#include <toml++/toml.hpp>
#include <imgui_internal.h>
using namespace mui;

// Forward declarations for UI creation functions
IControlPtr createBasicsTab(const LabelPtr &statusLabel);
IControlPtr createNumbersTab(const LabelPtr &statusLabel);
IControlPtr createTextEntriesTab(const LabelPtr &statusLabel);
IControlPtr createDialogsTab(const WindowPtr &parentWindow, const LabelPtr &statusLabel);
IControlPtr createLayoutsTab(const LabelPtr &statusLabel);
IControlPtr createBoxSizingTab(const LabelPtr &statusLabel);
IControlPtr createMoreControlsTab(const LabelPtr &statusLabel);
IControlPtr createAdvancedTab(const LabelPtr &statusLabel);
IControlPtr createPropertyGridTab(const LabelPtr &statusLabel);
IControlPtr createPropertyGridShowcaseTab(const LabelPtr &statusLabel);
IControlPtr createImageStackTab();
IControlPtr createThemesTab();
IControlPtr createShadowsTab();
WindowPtr createMainGalleryWindow();
WindowPtr createInspectorWindow();

int main()
{
    App::init("MUI Control Gallery", 800, 600, true, true);
    App::setTheme("Rounded Visual Studio");

    // ==========================================
    // STYLESHEET CONFIGURATION
    // ==========================================

    // Load styles from an external TOML file. This is more flexible than defining
    // them in C++. Make sure 'stylesheet.toml' is in the same directory as the
    // executable.
    StyleSheet::loadFromFile(App::getFilepath() + "stylesheet.toml");

    // Define a docking layout for our windows to arrange them on startup.
    App::setLayoutBuilder(
        [](mui::DockBuilder &builder)
        {
            ImGuiID dockspace_id = builder.getID();

            // Start with the full dockspace ID. After the split, this ID will refer to the left node.
            ImGuiID center_node_id = dockspace_id;
            ImGuiID right_node_id = builder.splitNode(center_node_id, mui::DockDirection::Right, 0.30f);

            // Dock windows. This only needs to be done once.
            builder.dockWindow("MUI Control Gallery", center_node_id);
            builder.dockWindow("Inspector", right_node_id);
        });

    // Create and show the main window with the control gallery
    auto mainWin = createMainGalleryWindow();
    mainWin->setNoTabBar(true);
    mainWin->show();

    // Create and show a second, independent "Inspector" window
    auto inspectorWin = createInspectorWindow();
    inspectorWin->setNoTabBar(true);
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
    auto lblStatus = UI::Label("Waiting for action...");

    auto win = UI::Window("MUI Control Gallery", 800, 600)
                   ->setMargined(true)
                   ->setScrollbar(false)
                   ->onClosing(
                       []()
                       {
                           App::quit();
                           return true;
                       });

    win << (UI::VBox(false)
            << Stretch(
                   UI::Tab()->setFittingPolicyScroll(false)->setFittingPolicyResizeDown(true)
                   << TabPage("Basics", createBasicsTab(lblStatus))
                   << TabPage("Numbers", createNumbersTab(lblStatus))
                   << TabPage("Text", createTextEntriesTab(lblStatus))
                   << TabPage("Dialogs", createDialogsTab(win, lblStatus))
                   << TabPage("Layouts", createLayoutsTab(lblStatus))
                   << TabPage("Box Sizing", createBoxSizingTab(lblStatus))
                   << TabPage("Properties", createPropertyGridTab(lblStatus))
                   << TabPage("Properties 2", createPropertyGridShowcaseTab(lblStatus))
                   << TabPage("Advanced", createAdvancedTab(lblStatus))
                   << TabPage("More", createMoreControlsTab(lblStatus))
                   << TabPage("Images", createImageStackTab())
                   << TabPage("Themes", createThemesTab())
                   << TabPage("Shadows", createShadowsTab()))
            << UI::Separator(SeparatorType::Native)
            << lblStatus);

    return win;
}

/**
 * @brief Creates a secondary window to demonstrate Cards, TreeNodes,
 * and multi-window capabilities.
 */
WindowPtr createInspectorWindow()
{
    auto win = UI::Window("Inspector", 300, 400)->setMargined(true);

    // --- Theme Controls ---
    auto btnPrev = UI::Button(ICON_FA_ARROW_LEFT " Prev")->addClass("nav_btn");
    auto btnNext = UI::Button(ICON_FA_ARROW_RIGHT " Next")->addClass("nav_btn");
    auto btnRandom = UI::Button(ICON_FA_SHUFFLE " Random")->addClass("nav_btn");
    auto lblTheme = UI::Label("Current Theme: " + App::getCurrentThemeName())->setSpanAvailWidth(true);

    // Logic for theme switching
    static std::vector<std::string> allThemes;
    if (allThemes.empty())
    {
        allThemes.push_back("Light");
        allThemes.push_back("Dark");
        auto tomlThemes = App::getAvailableThemes();
        allThemes.insert(allThemes.end(), tomlThemes.begin(), tomlThemes.end());
    }

    static int currentThemeIndex = -1;
    if (currentThemeIndex == -1)
    {
        // Find initial theme
        std::string current;
        if (App::isUsingTomlTheme())
        {
            current = App::getCurrentThemeName();
        }
        else
        {
            current = (App::getTheme() == ThemeType::Dark ? "Dark" : "Light");
        }
        auto it = std::find(allThemes.begin(), allThemes.end(), current);
        if (it != allThemes.end())
        {
            currentThemeIndex = std::distance(allThemes.begin(), it);
        }
        else
        {
            currentThemeIndex = 0; // Default to first if not found
        }
    }

    auto applyTheme = [lblTheme](int index)
    {
        if (index >= 0 && index < allThemes.size())
        {
            currentThemeIndex = index;
            const auto &themeName = allThemes[currentThemeIndex];
            if (themeName == "Light")
            {
                App::setTheme(ThemeType::Light);
                lblTheme->setText("Current Theme: " + themeName);
            }
            else if (themeName == "Dark")
            {
                App::setTheme(ThemeType::Dark);
                lblTheme->setText("Current Theme: " + themeName);
            }
            else
            {
                App::setTheme(themeName);
                lblTheme->setText("Current Theme: " + themeName);
            }
        }
    };

    btnPrev->onClick(
        [applyTheme]()
        {
            int newIndex = (currentThemeIndex - 1 + allThemes.size()) % allThemes.size();
            applyTheme(newIndex);
        });

    btnNext->onClick(
        [applyTheme]()
        {
            int newIndex = (currentThemeIndex + 1) % allThemes.size();
            applyTheme(newIndex);
        });

    btnRandom->onClick(
        [applyTheme]()
        {
            if (allThemes.size() > 1)
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distrib(0, allThemes.size() - 1);
                int newIndex = currentThemeIndex;
                while (newIndex == currentThemeIndex)
                {
                    newIndex = distrib(gen);
                }
                applyTheme(newIndex);
            }
        });

    // --- Card Example ---
    auto card = UI::Card()
                    ->setFillHeight(false)
                    ->setSpanAvailWidth(true)
                << (UI::VBox(true)
                    << UI::Label("This is a Card")
                    << UI::Separator(SeparatorType::Custom)
                    << UI::Label("This card is set to fill available width. The text below is wrapped to fit.")
                           ->setFormat(LabelFormat::Wrapped));

    // --- TreeNode Example ---
    auto tree = UI::TreeNode(ICON_FA_FOLDER " Project Files")
                    ->setSpanAvailWidth(true)
                << (UI::Label(ICON_FA_FILE_CODE " main.cpp")
                        ->setSpanAvailWidth(true))
                << (UI::TreeNode(ICON_FA_FOLDER " core")
                        ->setSpanAvailWidth(true)
                    << (UI::Button(ICON_FA_FILE_CODE " app.cpp")
                            ->setSpanAvailWidth(true))
                    << (UI::Label(ICON_FA_FILE_CODE " window.cpp")
                            ->setSpanAvailWidth(true)))
                << (UI::Label(ICON_FA_FILE_IMAGE " icon.png")
                        ->setSpanAvailWidth(true));

    // --- Main Layout ---
    win << (UI::VBox(false)
            << (UI::Group("Theme Controls")
                    ->setMargined(true)
                << (UI::FlowBox(mui::FlowBox::Align::Center)
                        ->setPadded(true)
                    << btnPrev
                    << lblTheme
                    << btnRandom
                    << btnNext))
            << card
            << UI::Separator(SeparatorType::Native)
            << Stretch(tree));

    return win;
}

IControlPtr createBasicsTab(const LabelPtr &lblStatus)
{
    auto btnClick =
        UI::Button(
            ICON_FA_FLOPPY_DISK " Click Me",
            [lblStatus]()
            {
                auto currentTime = std::time(nullptr);
                char timeStr[100];
                std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&currentTime));
                lblStatus->setText(std::string("Button clicked at ") + timeStr);
            })
            ->setID("btn_click");
    auto chkToggle =
        UI::Checkbox(
            "Standard Checkbox",
            false,
            [lblStatus](bool checked)
            {
                lblStatus->setText(checked ? "Checkbox Enabled" : "Checkbox Disabled");
            })
            ->setScale(0.6f); // Make the checkbox larger for better visibility

    // Modern ToggleSwitch demonstration
    auto modernToggle =
        UI::ToggleSwitch(
            "Modern iOS-Style Toggle",
            false,
            [lblStatus](bool state)
            {
                lblStatus->setText(state ? "Modern Toggle is ON" : "Modern Toggle is OFF");
            })
            ->setScale(0.6f); // Make the toggle larger for better visibility

    auto btnFindNode =
        UI::Button(
            ICON_FA_MAGNIFYING_GLASS " Find Inspector Dock Node",
            [lblStatus]()
            {
                const char *window_title = "Inspector";
                // Use ImGui's internal API to find the window by its title.
                ImGuiWindow *window = ImGui::FindWindowByName(window_title);

                // If the window is found and it is docked, get its DockNode.
                if (window && window->DockNode)
                {
                    ImGuiDockNode *node = window->DockNode;
                    char buffer[256];
                    snprintf(buffer, sizeof(buffer), "Inspector is in DockNode ID: 0x%08X. Tabs in this node: %d", node->ID, node->TabBar ? node->TabBar->Tabs.Size : 0);
                    lblStatus->setText(buffer);
                }
                else
                {
                    lblStatus->setText("Could not find a docked 'Inspector' window.");
                }
            });

    return UI::VBox(true, true)
           << UI::Label("Media Controls (IconStack):")
           << (UI::IconStack()
               << IconDef(
                      ICON_FA_PLAY,
                      [lblStatus]()
                      {
                          lblStatus->setText("Play clicked");
                      },
                      "Play Action", true)
               << IconDef(
                      ICON_FA_PAUSE,
                      [lblStatus]()
                      {
                          lblStatus->setText("Pause clicked");
                      },
                      "Pause Action")
               << IconDef(
                      ICON_FA_STOP,
                      [lblStatus]()
                      {
                          lblStatus->setText("Stop clicked");
                      },
                      "Stop Action"))
           << UI::Separator(SeparatorType::Native) << btnClick << chkToggle << modernToggle << UI::Separator(SeparatorType::Native) << Stretch(btnFindNode);
}

IControlPtr createNumbersTab(const LabelPtr &lblStatus)
{
    auto valueObservable = MakeObs::Int(50);
    auto progressBar = UI::ProgressBar(0.0f, "");

    // Manually trigger the update for the initial state
    progressBar->setValue(static_cast<float>(valueObservable->get()) / 100.0f);
    lblStatus->setText("Numeric controls synced to: " + std::to_string(valueObservable->get()));

    return UI::VBox(true, true)->setFillWidth(true)
           << UI::Label("Sync Test:")
           << (UI::SpinboxBind(0, 100, valueObservable)
               << UI::Observe(
                      valueObservable->onValueChanged,
                      [lblStatus, progressBar](int value)
                      {
                          progressBar->setValue(static_cast<float>(value) / 100.0f);
                          lblStatus->setText("Numeric controls synced to: " + std::to_string(value));
                      }))
           << UI::SliderIntBind(0, 100, valueObservable)
           << progressBar
           << UI::Separator(SeparatorType::Native)
           << UI::Label("Dual Handle Range Slider:")
           << (UI::RangeSlider(
                  0.0f, 100.0f, 20.0f, 80.0f,
                  [lblStatus](float vMin, float vMax)
                  {
                      char buf[128];
                      snprintf(buf, sizeof(buf), "Range changed: %.1f to %.1f", vMin, vMax);
                      lblStatus->setText(buf);
                  }));
}

IControlPtr createTextEntriesTab(const LabelPtr &lblStatus)
{
    auto textObservable = MakeObs::String("");
    auto searchObservable = MakeObs::String("");
    auto lblMirror = UI::Label("Mirror: ");

    lblMirror->setText("Mirror: " + textObservable->get()); // Initial value
    auto entry = UI::EntryBind("Type here...", textObservable)->setSpanAvailWidth(true);

    return UI::VBox(true, true)
           << UI::Label("Standard Entry:")
           << (entry
               << UI::Observe(
                      textObservable->onValueChanged,
                      [lblMirror, entry](const std::string &text)
                      {
                          if (text.empty())
                              entry->setInvalid(true);
                          else
                              entry->setInvalid(false);
                          lblMirror->setText("Mirror: " + text);
                      }))
           << UI::Label("Password Entry:")
           << (UI::PasswordEntry("Secret...")->setSpanAvailWidth(true))
           << UI::Label("Search Entry:")
           << (UI::SearchEntry("Search...")->setSpanAvailWidth(true)->bind(searchObservable)
               << UI::Observe(
                      searchObservable->onValueChanged,
                      [lblStatus](const std::string &text)
                      {
                          if (!text.empty())
                          {
                              lblStatus->setText("Searching for: " + text);
                          }
                      }))
           << lblMirror;
}

IControlPtr createDialogsTab(const WindowPtr &win, const LabelPtr &lblStatus)
{
    auto btnInfo = UI::Button(
                       ICON_FA_CIRCLE_INFO " Info",
                       [win, lblStatus]()
                       {
                           Dialogs::msgBoxInfo("Information", "This is an informational message box, which is the default type.");
                           lblStatus->setText("Info dialog shown.");
                       })
                       ->addClass("info");

    auto btnWarning = UI::Button(
                          ICON_FA_TRIANGLE_EXCLAMATION " Warning",
                          [win, lblStatus]()
                          {
                              Dialogs::msgBoxWarning("Warning", "This is a warning message. Something might be wrong, so you should pay attention.");
                              lblStatus->setText("Warning dialog shown.");
                          })
                          ->addClass("warning");

    auto btnError = UI::Button(
                        ICON_FA_CIRCLE_XMARK " Error",
                        [win, lblStatus]()
                        {
                            Dialogs::msgBoxError("Critical Error", "Failed to load imaginary resources.");
                            lblStatus->setText("Error dialog shown.");
                        })
                        ->addClass("danger");

    auto btnConfirm = UI::Button(
                          ICON_FA_CIRCLE_CHECK " Confirm",
                          [win, lblStatus]()
                          {
                              Dialogs::msgBoxConfirm(
                                  "Confirmation",
                                  "This action is permanent. Are you sure you want to proceed?",
                                  [lblStatus]()
                                  {
                                      lblStatus->setText("Confirmed!");
                                  },
                                  [lblStatus]()
                                  {
                                      lblStatus->setText("Cancelled.");
                                  });
                          })
                          ->addClass("success");

    auto btnQuestion = UI::Button(
        ICON_FA_CIRCLE_QUESTION " Question",
        [win, lblStatus]()
        {
            Dialogs::msgBoxQuestion(
                "Unsaved Work",
                "Do you want to save your changes before closing?",
                [lblStatus]()
                {
                    lblStatus->setText("Answered: Yes");
                },
                [lblStatus]()
                {
                    lblStatus->setText("Answered: No");
                });
        });

    auto btnCustom = UI::Button(
        ICON_FA_GEARS " Custom",
        [lblStatus]()
        {
            Dialogs::msgBoxCustom(
                "File Operation", "The destination file already exists.",
                MessageBoxType::Warning,
                {{"Overwrite",
                  [lblStatus]()
                  {
                      lblStatus->setText("File Overwritten.");
                  }},
                 {"Skip",
                  [lblStatus]()
                  {
                      lblStatus->setText("File Skipped.");
                  }},
                 {"Cancel",
                  [lblStatus]()
                  {
                      lblStatus->setText("Operation Cancelled.");
                  }}});
        });

    auto btnFile = UI::Button(
        ICON_FA_FOLDER_OPEN " Open File",
        [win, lblStatus]()
        {
            Dialogs::openFile(
                "Open File",
                "All Files {*.*}",
                [lblStatus](const std::string &path)
                {
                    Dialogs::msgBox("File Selected", "Path: " + path);
                    lblStatus->setText("File selected: " + path);
                },
                [lblStatus]()
                {
                    lblStatus->setText("File open dialog was cancelled.");
                });
        });

    return UI::VBox(true, true)
           << (UI::Group("System Dialogs")
                   ->setMargined(true)
               << (UI::FlowBox(mui::FlowBox::Align::Justify)
                       ->setPadded(true)
                   << btnInfo
                   << btnWarning
                   << btnError
                   << btnConfirm
                   << btnQuestion
                   << btnCustom
                   << btnFile))
           << UI::Separator()
           << (UI::Group("Inline Path Picker")
                   ->setMargined(true)
               << (UI::VBox(true)
                   << mui::PathPicker::create()
                          ->setMode(mui::PathPickerMode::Folder)
                          ->setSpanAvailWidth(true)
                          ->onPathChanged([lblStatus](const std::string &path)
                                          { lblStatus->setText("PathPicker selected: " + path); })
                   << mui::PathPicker::create()
                          ->setMode(mui::PathPickerMode::File)
                          ->setShowFavorites(false)
                          ->setShowNewFolder(false)
                          ->setSpanAvailWidth(true)
                          ->onPathChanged([lblStatus](const std::string &path)
                                          { lblStatus->setText("PathPicker selected: " + path); })))
           << UI::Separator()
           << (UI::Group("Group Style Variants")
                   ->setMargined(true)
                   ->defaultOpen(true)
               << (UI::VBox(true)
                   << (UI::Group("Open on Double-Click")
                           ->setOpenOnDoubleClick(true)
                       << UI::Label("This group opens when you double-click the header."))
                   << (UI::Group("Open on Arrow Click Only")
                           ->setOpenOnArrow(true)
                       << UI::Label("This group only opens when you click the arrow icon."))
                   << (UI::Group("Bullet Style (no arrow)")
                           ->setBullet(true)
                       << UI::Label("This group uses a bullet instead of an arrow."))
                   << (UI::Group("Spans Available Width (entire header is clickable)")
                           ->setSpanAvailWidth(true)
                       << UI::Label("The entire width of this header is clickable."))));
}

IControlPtr createLayoutsTab(const LabelPtr &lblStatus)
{
    auto firstNameEntry = UI::Entry("John")->setSpanAvailWidth(true);
    auto lastNameEntry = UI::Entry("Doe")->setSpanAvailWidth(true);
    auto addressEntry = UI::Entry("123 Main St, Anytown")->setSpanAvailWidth(true);
    auto submitButton = UI::Button(
        "Submit",
        [lblStatus, firstNameEntry, lastNameEntry, addressEntry]()
        {
            std::string text = "Submitted: " + firstNameEntry->getText() + " " + lastNameEntry->getText() + " from " + addressEntry->getText();
            lblStatus->setText(text);
        });

    auto grid = UI::Grid({0.3f, 0.7f})
                << GridCell(UI::Label("First Name:"), 0, 0)
                << GridCell(firstNameEntry, 0, 1)
                << GridCell(UI::Label("Last Name:"), 1, 0)
                << GridCell(lastNameEntry, 1, 1)
                << GridCell(UI::Label("Address:"), 2, 0)
                << GridCell(addressEntry, 2, 1)
                << GridCell(UI::Label(""), 3, 0)
                << GridCell(submitButton, 3, 1);

    auto vbox = UI::VBox(true, true)
                << (UI::Group("Grid Layout")->setMargined(true) << grid)
                << UI::Separator(SeparatorType::Native);

    // Flow layouts
    auto flowGroup = UI::Group("Flow Layout (Left)")->setMargined(true);
    auto flowBox = UI::FlowBox(mui::FlowBox::Align::Left);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Button " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " is a bit longer";
        }
        flowBox
            << UI::Button(
                   label,
                   [lblStatus, label]()
                   {
                       lblStatus->setText(label + " clicked");
                   });
    }
    flowGroup << flowBox;
    vbox << flowGroup;

    auto flowGroupCenter = UI::Group("Flow Layout (Center)")->setMargined(true);
    auto flowBoxCenter = UI::FlowBox(mui::FlowBox::Align::Center);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Button " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " is a bit longer";
        }
        flowBoxCenter
            << UI::Button(
                   label,
                   [lblStatus, label]()
                   {
                       lblStatus->setText(label + " clicked");
                   });
    }
    flowGroupCenter << flowBoxCenter;
    vbox << flowGroupCenter;

    auto flowGroupRight = UI::Group("Flow Layout (Right)")->setMargined(true);
    auto flowBoxRight = UI::FlowBox(mui::FlowBox::Align::Right);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Button " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " is a bit longer";
        }
        flowBoxRight
            << UI::Button(
                   label,
                   [lblStatus, label]()
                   {
                       lblStatus->setText(label + " clicked");
                   });
    }
    flowGroupRight << flowBoxRight;
    vbox << flowGroupRight;

    auto flowGroupJustify = UI::Group("Flow Layout (Justify)")->setMargined(true);
    auto flowBoxJustify = UI::FlowBox(mui::FlowBox::Align::Justify);
    for (int i = 0; i < 15; ++i)
    {
        std::string label = "Btn " + std::to_string(i + 1);
        if (i % 4 == 0)
        {
            label += " long";
        }
        flowBoxJustify
            << UI::Button(
                   label,
                   [lblStatus, label]()
                   {
                       lblStatus->setText(label + " clicked");
                   });
    }
    flowGroupJustify << flowBoxJustify;
    vbox << flowGroupJustify;

    return vbox;
}

IControlPtr createBoxSizingTab(const LabelPtr &statusLabel)
{
    // --- VBox Demo ---
    // Create a VBox with children using all sizing modes.
    // setFillWidth(true) makes the children (Buttons) expand to fill the width of the VBox.
    auto vbox_demo = UI::VBox(true)->setSpacing(5.0f)->setFillWidth(true)->setShowChildScrollbars(true);
    vbox_demo->append(UI::Button("Fixed Height: 50px"), Sizing::Fixed(50.0f));
    vbox_demo->append(UI::Button("Percent Height: 25%"), Sizing::Percent(25.0f));
    vbox_demo->append(UI::Button("Auto Height"), Sizing::Auto());
    vbox_demo->append(UI::Button("Stretch Weight 1.0"), Sizing::Stretch(1.0f));
    vbox_demo->append(UI::Button("Stretch Weight 2.0"), Sizing::Stretch(2.0f));

    // --- HBox Demo ---
    // Create an HBox with children using all sizing modes.
    // setFillHeight(true) makes the children (Buttons) expand to fill the height of the HBox.
    auto hbox_demo = UI::HBox(true)->setSpacing(5.0f)->setFillHeight(true)->setShowChildScrollbars(true);
    hbox_demo->append(UI::Button("Fixed\n50px"), Sizing::Fixed(50.0f));
    hbox_demo->append(UI::Button("Percent\n25%"), Sizing::Percent(25.0f));
    hbox_demo->append(UI::Button("Auto"), Sizing::Auto());
    hbox_demo->append(UI::Button("Stretch\n1.0"), Sizing::Stretch(1.0f));
    hbox_demo->append(UI::Button("Stretch\n2.0"), Sizing::Stretch(2.0f));

    // --- FlowBox Demo ---
    // FlowBox naturally wraps items. We can combine sizing modes to create responsive grids.
    auto flow_demo = UI::FlowBox(mui::FlowBox::Align::Left)->setSpacing(5.0f)->setFillHeight(true)->setShowChildScrollbars(true);
    flow_demo->append(UI::Button("Fixed Width\n150px"), Sizing::Fixed(150.0f));
    flow_demo->append(UI::Button("Percent Width\n30%"), Sizing::Percent(30.0f));
    flow_demo->append(UI::Button("Auto Width"), Sizing::Auto());
    flow_demo->append(UI::Button("Stretch\nWeight 1.0"), Sizing::Stretch(1.0f));
    flow_demo->append(UI::Button("Stretch\nWeight 2.0"), Sizing::Stretch(2.0f));
    flow_demo->append(UI::Button("Another Auto"), Sizing::Auto());

    auto top_split = UI::Splitter(SplitterOrientation::Horizontal, 0.5f)
                     << SplitterLeft(
                            UI::Group("VBox Sizing Demo")
                                ->setMargined(true)
                            << (UI::VBox(false) << Stretch(vbox_demo)))
                     << SplitterRight(
                            UI::Group("HBox Sizing Demo")
                                ->setMargined(true)
                            << (UI::VBox(false) << Stretch(hbox_demo)));

    auto main_split = UI::Splitter(SplitterOrientation::Vertical, 0.66f)
                      << SplitterLeft(top_split)
                      << SplitterRight(
                             UI::Group("FlowBox Sizing Demo")
                                 ->setMargined(true)
                             << (UI::VBox(false) << Stretch(flow_demo)));

    // The main layout for this tab will be a splitter to give defined areas to the demos.
    return UI::VBox(true, true)
           << UI::Label("Demonstrates the different sizing modes for children in VBox, HBox, and FlowBox layouts. Resize the window or splitters to see how 'Stretch' and 'Percent' modes adapt. Child scrollbars are explicitly enabled to handle overflow.")->setFormat(LabelFormat::Wrapped)
           << UI::Separator()
           << Stretch(main_split);
}

IControlPtr createAdvancedTab(const LabelPtr &lblStatus)
{
    // Breadcrumb Navigator
    auto breadcrumb = UI::BreadcrumbBar(
        "C:/Users/Public/Documents",
        [lblStatus](const std::string &path)
        {
            lblStatus->setText("Navigated to: " + path);
        });

    // Left Panel: Directory Tree
    auto leftPanel = UI::VBox(true, true)
                     << Stretch(
                            UI::TreeNode(ICON_FA_DESKTOP " This PC")
                                ->setDefaultOpen(true)
                            << UI::Label(ICON_FA_HARD_DRIVE " Local Disk (C:)")
                            << (UI::TreeNode(ICON_FA_FOLDER " Data (D:)")
                                << UI::Label(ICON_FA_FOLDER " Backups")
                                << UI::Label(ICON_FA_FOLDER " Projects")));

    // Right Panel: Data Table
    auto table = UI::Table({{"Name", 3.0f}, {"Date Modified", 2.0f}, {"Size", 1.0f}})
                     ->setSortable(true)
                     ->setSpanAvailWidth(true);

    // Add some mock file data
    table << TableRow({UI::Label(ICON_FA_FOLDER " Photos"), UI::Label("10/24/2023"), UI::Label("--")})
          << TableRow({UI::Label(ICON_FA_FOLDER " Videos"), UI::Label("10/12/2023"), UI::Label("--")})
          << TableRow({UI::Label(ICON_FA_FILE_WORD " Q4_Report.docx"), UI::Label("10/25/2023"), UI::Label("2,450 KB")})
          << TableRow({UI::Label(ICON_FA_FILE_EXCEL " Budget_2024.xlsx"), UI::Label("10/26/2023"), UI::Label("124 KB")})
          << TableRow({UI::Label(ICON_FA_FILE_PDF " Invoice_992.pdf"), UI::Label("10/27/2023"), UI::Label("89 KB")});

    table->onRowSelected(
        [lblStatus](int row)
        {
            lblStatus->setText("Selected table row: " + std::to_string(row));
        });

    table->onSortRequested(
        [lblStatus](int colIndex, bool ascending)
        {
            std::string dir = ascending ? "Ascending" : "Descending";
            lblStatus->setText("Sort requested on column " + std::to_string(colIndex) + " (" + dir + ")");
        });

    auto rightPanel = UI::VBox(true) << Stretch(table);

    return UI::VBox(false)
           << (UI::VBox(true)
               << (UI::HBox(true)
                   << (UI::IconStack()
                       << IconDef(
                              ICON_FA_ARROW_LEFT,
                              [lblStatus]()
                              {
                                  lblStatus->setText("Back clicked");
                              },
                              "Go Back")
                       << IconDef(
                              ICON_FA_ARROW_RIGHT,
                              [lblStatus]()
                              {
                                  lblStatus->setText("Forward clicked");
                              },
                              "Go Forward")
                       << IconDef(
                              ICON_FA_ARROW_UP,
                              [lblStatus]()
                              {
                                  lblStatus->setText("Up clicked");
                              },
                              "Up to Parent Folder"))
                   << Stretch(breadcrumb))
               << UI::Separator(SeparatorType::Native))
           << Stretch(UI::Splitter(SplitterOrientation::Horizontal, 0.25f) << SplitterLeft(leftPanel) << SplitterRight(rightPanel));
}

IControlPtr createPropertyGridTab(const LabelPtr &statusLabel)
{
    auto nameObservable = MakeObs::String("My Widget");
    auto nameEntry = UI::EntryBind("", nameObservable)
                     << UI::Observe(
                            nameObservable->onValueChanged,
                            [statusLabel](const std::string &text)
                            {
                                statusLabel->setText("Name changed to: " + text);
                            });

    auto comboObservable = MakeObs::Int(0);
    auto modeCombo = UI::ComboBoxBind(comboObservable)
                     << "Mode A" << "Mode B" << "Mode C";
    modeCombo->observe(
        comboObservable->onValueChanged,
        [modeCombo, statusLabel](int)
        {
            statusLabel->setText("Mode set to: " + modeCombo->getText());
        });

    return UI::VBox(true, true)
           << Stretch(
                  UI::PropertyGrid(120.0f)
                  << PropCat("Appearance " ICON_FA_PALETTE)
                  << PropItem("Name", nameEntry)
                  << PropItem(
                         "Visible",
                         UI::Checkbox(
                             "", true,
                             [statusLabel](bool checked)
                             {
                                 statusLabel->setText(checked ? "Set to Visible" : "Set to Hidden");
                             }))
                  << PropItem("Color", UI::ColorEdit(0.9f, 0.2f, 0.2f, 1.0f))
                  << PropItem("Opacity", UI::SliderFloat(0.0f, 1.0f, 1.0f))
                  << PropCat("Behavior " ICON_FA_BOLT)
                  << PropItem(
                         "Enabled",
                         UI::ToggleSwitch(
                             "", true,
                             [statusLabel](bool checked)
                             {
                                 statusLabel->setText(checked ? "Control Enabled" : "Control Disabled");
                             }))
                  << PropItem("Mode", modeCombo)
                  << PropCat("Advanced " ICON_FA_GEAR, false)
                  << PropItem("ID", UI::Label("0xDEADBEEF")->setColor({0.6f, 0.6f, 0.6f, 1.0f}))
                  << PropItem("File Path", mui::PathPicker::create()->setPath("C:/path/to/resource.dat")->setSpanAvailWidth(true)));
}

IControlPtr createPropertyGridShowcaseTab(const LabelPtr &statusLabel)
{
    // State for the advanced property grid
    static auto fontSizeObs = MakeObs::Float(13.f);
    static auto healthObs = MakeObs::Float(75.f);
    static auto manaObs = MakeObs::Float(55.f);
    static auto isInvincibleObs = MakeObs::Bool(false);
    static auto godModeObs = MakeObs::Bool(false);
    static auto debugFlagsObs = MakeObs::Int(0);
    static auto themeObs = MakeObs::Int(0);

    auto pg = UI::PropertyGrid(180.f)           // Set name column width
              << PGValueColumnWeight{1.5f}      // Make value column wider
              << PGItemSpacingY{4.f}            // Add some vertical space between items
              << PGCategorySpacingY{8.f}        // And between categories
              << PGItemIndent{25.f}             // A bit more indentation
              << PGSpanCategoryHeaders{true}    // Make headers span both columns
              << PGZebraStripes{false};         // Disable zebra stripes for this showcase
                  
    // Category with Icon and Color
    pg << PropCat("Appearance " ICON_FA_BRUSH, ICON_FA_BRUSH, ImVec4(0.2f, 0.6f, 0.9f, 1.0f))
       << PropItem("Theme", UI::ComboBoxBind(themeObs) << "Dark" << "Light" << "Classic")
       << PropItem("Font Size", UI::SliderFloatBind(8.f, 24.f, fontSizeObs));

    // Category with custom label colors and different flags
    pg << PropCat(("Status " ICON_FA_HEART_PULSE), "", ImVec4(0, 0, 0, 0), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)
       << PropItem("Health", UI::SliderFloatBind(0.f, 100.f, healthObs), {0.1f, 0.8f, 0.2f, 1.0f})
       << PropItem("Mana", UI::SliderFloatBind(0.f, 100.f, manaObs), {0.2f, 0.4f, 0.9f, 1.0f})
       << PropItem("Is Invincible", UI::CheckboxBind("", isInvincibleObs), {0.9f, 0.7f, 0.1f, 1.0f});

    // A closed-by-default category
    pg << PropCat(("Advanced " ICON_FA_GEAR), "", ImVec4(0, 0, 0, 0), 0) // flags = 0 means not open by default
       << PropItem("Enable God Mode", UI::CheckboxBind("", godModeObs))
       << PropItem("Debug Flags", UI::SpinboxBind(0, 1000, debugFlagsObs));

    return UI::VBox(true, true)
           << Stretch(pg);
}

IControlPtr createMoreControlsTab(const LabelPtr &lblStatus)
{
    auto comboObservable = MakeObs::Int(1);
    auto combo = UI::ComboBoxBind(comboObservable)
                 << "Apple" << "Banana" << "Cherry";
    combo->observe(
        comboObservable->onValueChanged,
        [combo, lblStatus](int)
        {
            lblStatus->setText("Selected fruit: " + combo->getText());
        });

    auto radio1 = UI::RadioButton(
        "AM",
        false,
        [lblStatus](bool c)
        {
            if (c)
                lblStatus->setText("Radio set to AM");
        });
    auto radio2 = UI::RadioButton(
        "FM",
        true,
        [lblStatus](bool c)
        {
            if (c)
                lblStatus->setText("Radio set to FM");
        });
    auto radio3 = UI::RadioButton(
        "SW",
        false,
        [lblStatus](bool c)
        {
            if (c)
                lblStatus->setText("Radio set to SW");
        });
    RadioButton::group({radio1, radio2, radio3});

    auto colorCallback = [lblStatus](const std::array<float, 4> &val)
    {
        char buffer[100];
        snprintf(buffer, 100, "Color changed to: (%.2f, %.2f, %.2f, %.2f)", val[0], val[1], val[2], val[3]);
        lblStatus->setText(buffer);
    };
    auto colorObservable = MakeObs::Color(0.2f, 0.8f, 0.4f, 1.0f);
    auto colorEdit = UI::ColorEditBind(colorObservable)
                     << UI::Observe(
                            colorObservable->onValueChanged,
                            colorCallback);

    auto sliderObservable = MakeObs::Float(0.75f);
    auto sliderFloat = UI::SliderFloatBind(0.0f, 1.0f, sliderObservable)
                       << UI::Observe(
                              sliderObservable->onValueChanged,
                              [lblStatus](float value)
                              {
                                  lblStatus->setText("Float slider value: " + std::to_string(value));
                              });

    return UI::VBox(true, true)
           << UI::Label("ComboBox:")
           << Stretch(combo)
           << UI::Separator()
           << UI::Label("Radio Buttons:")
           << (UI::HBox(true)
               << Stretch(radio1)
               << Stretch(radio2)
               << Stretch(radio3))
           << UI::Separator()
           << UI::Label("Color Picker:")
           << Stretch(colorEdit)
           << UI::Separator()
           << UI::Label("Float Slider:")
           << Stretch(sliderFloat)
           << UI::Separator()
           << UI::Label("Custom Separators:")
           << (UI::HBox(true)
               << UI::Label("Left")
               << (UI::Separator(SeparatorType::Native, SeparatorOrientation::Vertical)
                       ->setThickness(4.0f)
                       ->setColor({0.8f, 0.2f, 0.2f, 1.0f}))
               << UI::Label("Right"))
           << UI::Separator()->setThickness(5.0f)->setAsRect(true)
           << UI::Separator(SeparatorType::Native)
           << UI::Separator(SeparatorType::Text)->setText("Text Separator");
}

IControlPtr createThemesTab()
{
    auto tglGrayscale = UI::ToggleSwitch(
        "Grayscale",
        false,
        [](bool c)
        {
            App::setApplyGrayscale(c);
        });
    auto tglComplementary = UI::ToggleSwitch(
        "Complementary",
        false,
        [](bool c)
        {
            App::setApplyComplementary(c);
        });
    auto tglSepia = UI::ToggleSwitch(
        "Sepia",
        false,
        [](bool c)
        {
            App::setApplySepia(c);
        });
    auto tglInvert = UI::ToggleSwitch(
        "Invert",
        false,
        [](bool c)
        {
            App::setApplyInvert(c);
        });

    // A lambda to reset all toggles. This is called when a new theme is selected,
    // as App::setTheme() resets all underlying modifier flags.
    auto reset_toggles = [=]()
    {
        tglGrayscale->setChecked(false);
        tglComplementary->setChecked(false);
        tglSepia->setChecked(false);
        tglInvert->setChecked(false);
    };

    auto tomlFlowBox = UI::FlowBox(mui::FlowBox::Align::Justify);
    for (const auto &themeName : App::getAvailableThemes())
    {
        tomlFlowBox << UI::Button(themeName, [themeName, reset_toggles]()
                                  {
            App::setTheme(themeName);
            reset_toggles(); });
    }

    return UI::VBox(true, true)
           << UI::Label("Built-in Themes:")
           << (UI::HBox(true)
               << Stretch(UI::Button(
                      "Light Theme",
                      [=]()
                      {
                          App::setTheme(ThemeType::Light);
                          reset_toggles();
                      }))
               << Stretch(UI::Button(
                      "Dark Theme",
                      [=]()
                      {
                          App::setTheme(ThemeType::Dark);
                          reset_toggles();
                      })))
           << UI::Label("Color Modifiers:")
           << (UI::HBox(true)
               << Stretch(tglGrayscale)
               << Stretch(tglComplementary)
               << Stretch(tglSepia)
               << Stretch(tglInvert))
           << UI::Button(
                  "Clear Modifiers",
                  [=]()
                  {
                      App::resetColorModifiers();
                      reset_toggles();
                  })
           << UI::Separator()
           << UI::Label("TOML Themes (from themes.toml):")
           << Stretch(tomlFlowBox);
}

IControlPtr createShadowsTab()
{
    auto btn1 = UI::Button("No Shadow")->addClass("no_shadow");
    auto btn2 = UI::Button("Default Shadow")->setShadow(true);
    auto btn3 = UI::Button("Custom Shadow")->setID("custom_shadow_btn");

    auto card1 = UI::Card()
                     ->setPadding(20.0f)
                     ->setID("shadow_card")
                 << UI::Label("Card with a large, soft drop shadow");

    return UI::VBox(true, true)
           << UI::Label("Widgets can have customizable drop shadows.")
           << UI::Separator()
           << (UI::FlowBox()
               << btn1
               << btn2
               << btn3)
           << UI::Separator()
           << card1;
}

IControlPtr createImageStackTab()
{
    auto stackView = mui::ImageStackView::create();

    const int width = 256;
    const int height = 256;
    std::vector<unsigned char> img1(width * height * 4);
    std::vector<unsigned char> img2(width * height * 4);
    std::vector<unsigned char> img3(width * height * 4);
    std::vector<unsigned char> img4(width * height * 4);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int i = (y * width + x) * 4;
            // Image 1: Red Gradient
            img1[i + 0] = 255;
            img1[i + 1] = 0;
            img1[i + 2] = 0;
            img1[i + 3] = (unsigned char)((x / (float)width) * 255.0f);

            // Image 2: Blue Checkerboard
            bool isWhite = ((x / 32) + (y / 32)) % 2 == 0;
            img2[i + 0] = 0;
            img2[i + 1] = 0;
            img2[i + 2] = 255;
            img2[i + 3] = isWhite ? 255 : 50;

            // Image 3: Green Circle
            float dx = x - width / 2.0f;
            float dy = y - height / 2.0f;
            float dist = std::sqrt(dx * dx + dy * dy);
            bool inCircle = dist < 100.0f;
            img3[i + 0] = 0;
            img3[i + 1] = 255;
            img3[i + 2] = 0;
            img3[i + 3] = inCircle ? 200 : 0;

            // Image 4: Yellow/Black Stripes
            bool isYellow = (x + y) % 64 < 32;
            img4[i + 0] = isYellow ? 255 : 0;
            img4[i + 1] = isYellow ? 255 : 0;
            img4[i + 2] = 0;
            img4[i + 3] = 128; // Semi-transparent
        }
    }

    stackView->loadFromMemory("Red Gradient", width, height, 4, img1.data());
    stackView->loadFromMemory("Blue Checkerboard", width, height, 4, img2.data());
    stackView->loadFromMemory("Green Circle", width, height, 4, img3.data());
    stackView->loadFromMemory("Yellow Stripes", width, height, 4, img4.data());

    return UI::VBox(true, true)
           << UI::Label("ImageStackView supports multiple layers, zooming (scroll), and panning (L/M/R click & drag).")
           << Stretch(stackView);
}