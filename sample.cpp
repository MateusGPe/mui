#include "mui.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

using namespace mui;

// ==========================================
// Application State (Observables)
// ==========================================
// Observables are thread-safe. When their value changes, they automatically
// trigger signals. We use them here to bridge the background worker with the UI.
auto obsStatus = std::make_shared<Observable<std::string>>("Idle");
auto obsProgress = std::make_shared<Observable<float>>(0.0f);
auto obsEnableAI = std::make_shared<Observable<bool>>(true);
auto obsSteps = std::make_shared<Observable<int>>(20);
auto obsCfgScale = std::make_shared<Observable<float>>(7.5f);

// ==========================================
// Background Worker Simulation
// ==========================================
void RunInferenceTask()
{
    // Update status (Thread-Safe)
    obsStatus->set("Initializing Engine...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    int totalSteps = obsSteps->get();

    for (int i = 1; i <= totalSteps; ++i)
    {
        // Simulate heavy computation (e.g., stable-diffusion.cpp step)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Update progress and status safely from the background thread
        obsProgress->set(static_cast<float>(i) / totalSteps);

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Processing step %d of %d...", i, totalSteps);
        obsStatus->set(buffer);
    }

    obsStatus->set("Generation Complete!");
}

// ==========================================
// UI Construction
// ==========================================
std::shared_ptr<mui::Window> BuildMainWindow(mui::ProgressBarPtr *pBar)
{
    auto mainWindow =
        UI::Window("Niflheim Engine GUI", 1280, 800)
        << (UI::Splitter(SplitterOrientation::Horizontal, 0.25f)

            // ------------------------------------------
            // LEFT PANEL: Sidebar Navigation
            // ------------------------------------------
            << SplitterLeft(
                   UI::Card(0.0f, true)
                   << (UI::VBox()
                       << UI::SearchEntry("Search models...")
                       << UI::Separator()

                       << (UI::TreeNode("Checkpoints", true, ICON_FA_FOLDER)
                           << UI::Label("sd-v1-5.safetensors", LabelFormat::Disabled)
                           << UI::Label("sdxl-base-1.0.safetensors", LabelFormat::Disabled))
                       << (UI::TreeNode("LoRAs", false, ICON_FA_FOLDER)
                           << UI::Label("add_detail.safetensors", LabelFormat::Disabled))))

            // ------------------------------------------
            // RIGHT PANEL: Main Editor & Parameters
            // ------------------------------------------
            << SplitterRight(
                   UI::VBox()
                   << UI::Label("Generation Parameters", LabelFormat::Bullet)
                   << UI::Separator()

                   // Prompts
                   << UI::Parameter("Prompt", UI::Entry("Enter positive prompt...")->setMultiline(true, 100))
                   << UI::Parameter("Negative", UI::Entry("Enter negative prompt...")->setMultiline(true, 60))

                   // Settings Grid
                   << (UI::Group("Sampler Configuration")
                       << UI::Parameter("Steps", UI::SpinboxBind(1, 150, obsSteps))
                       << UI::Parameter("CFG Scale", UI::SliderFloatBind(1.0f, 30.0f, obsCfgScale))
                       << UI::Parameter("Method", UI::ComboBox() << "Euler a" << "DPM++ 2M Karras" << "DDIM"))

                   // Stretchy Spacer to push the bottom bar down
                   << Stretch(UI::Separator(SeparatorType::Custom)->setThickness(0))

                   // ------------------------------------------
                   // BOTTOM ACTION BAR
                   // ------------------------------------------
                   << UI::Separator()
                   << (UI::HBox(true, false)

                       // Left side of action bar
                       << UI::ToggleSwitchBind("Hardware Accel", obsEnableAI)
                       << Stretch(UI::Separator(SeparatorType::Custom)->setThickness(0))

                       // Center/Right side status info
                       << UI::Label("Status:")
                       // Use LabelBind for read-only observable text.
                       << UI::LabelBind(obsStatus)->setWidth(200.0f)
                       // Add the pre-configured progress bar to the layout.
                       << *pBar
                       // Generate Button triggers the detached background worker
                       << UI::Button(
                              "Generate",
                              []()
                              {
                                  if (obsStatus->get() == "Idle" || obsStatus->get() == "Generation Complete!")
                                  {
                                      std::thread(RunInferenceTask).detach();
                                  }
                              })
                              ->setColor(ImVec4(0.2f, 0.6f, 0.2f, 1.0f), // Base Green
                                         ImVec4(0.3f, 0.7f, 0.3f, 1.0f), // Hover Green
                                         ImVec4(0.1f, 0.5f, 0.1f, 1.0f)) // Active Green
                       )));

    // Ensure the window stops the application when closed
    mainWindow->onClosing(
        []() -> bool
        {
            // Connections are now managed by the widgets, so no manual cleanup is needed.
            mui::App::quit();
            return true;
        });

    return mainWindow;
}

// ==========================================
// Application Entry Point
// ==========================================
int main(int argc, char *argv[])
{
    try
    {
        // 1. Initialize the App Engine
        // Pass `true` if you want the OpenGL backend, `false` for SDL Renderer
        mui::App::init(true);

        // 2. Configure global styling
        mui::App::setTheme(mui::ThemeType::Dark);

        mui::ProgressBarPtr pBar;
        pBar = UI::ProgressBar(0.0f, "Ready")->setWidth(200.0f);
        pBar << UI::Observe(
            obsProgress->onValueChanged,
            [pBar](float val)
            {
                App::queueMain(
                    [pBar, val]()
                    {
                        pBar->setValue(val)->setOverlayText(val >= 1.0f ? "Done" : "");
                    });
            });
        auto window = BuildMainWindow(&pBar);

        // 4. Start the blocking UI event loop
        mui::App::run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return -1;
    }

    // 5. Cleanup
    mui::App::shutdown();
    return 0;
}

/* #include "mui.hpp"
#include <iostream>

using namespace mui;

int main() {
    // 1. Initialization
    App::init(true); // Initialize with OpenGL

    // 2. Main Window Creation
    auto mainWindow = Window::create("MUI Exhaustive Widget Gallery", 1280, 800)
        ->setMargined(true)
        ->setDocking(true)
        ->setCollapsible(true)
        ->setResizable(true);

    // Main layout container
    auto mainTab = Tab::create()
        ->setReorderable(true)
        ->setAutoSelectNewTabs(true)
        ->setFittingPolicyScroll(true);

    // ==========================================
    // PAGE 1: Buttons & Toggles
    // ==========================================
    auto btnBox = VBox::create()->setPadded(true)->setScrollable(true);

    auto standardGrp = Group::create("Standard Buttons")->setMargined(true);
    auto standardFlow = FlowBox::create()->setAlign(FlowBox::Align::Left);
    standardFlow->append({
        {Button::create("Normal Button")->setType(ButtonType::Normal), false},
        {Button::create("Small Button")->setType(ButtonType::Small), false},
        {Button::create("Arrow Left")->setType(ButtonType::ArrowLeft), false},
        {Button::create("Arrow Right")->setType(ButtonType::ArrowRight), false}
    });
    standardGrp->setChild(standardFlow);
    btnBox->append(standardGrp);

    auto iconBtnGrp = Group::create("Icon & Composite Buttons")->setMargined(true);
    auto iconHBox = HBox::create()->setPadded(true);
    iconHBox->append({
        {IconButton::create("Save")->setIconText("ICON_SAVE")->setLayout(IconButtonLayout::Horizontal), false},
        {IconButton::create("Delete")->setIconText("ICON_TRASH")->setLayout(IconButtonLayout::Vertical)->setSelected(true), false}
    });
    iconBtnGrp->setChild(iconHBox);
    btnBox->append(iconBtnGrp);

    auto toggleGrp = Group::create("Checkboxes, Radios & Switches")->setMargined(true);
    auto toggleVBox = VBox::create()->setPadded(true);

    auto r1 = RadioButton::create("Option A")->setChecked(true);
    auto r2 = RadioButton::create("Option B");
    auto r3 = RadioButton::create("Option C");
    RadioButton::group({r1, r2, r3});

    toggleVBox->append({
        {Checkbox::create("Standard Checkbox")->setChecked(true)->setScale(1.2f), false},
        {ToggleSwitch::create("Enable Feature X")->setChecked(true)->setScale(1.0f), false},
        {ToggleSwitch::create("Enable Feature Y")->setChecked(false)->setScale(1.5f), false},
        {Separator::create()->setOrientation(SeparatorOrientation::Horizontal), false},
        {r1, false}, {r2, false}, {r3, false}
    });
    toggleGrp->setChild(toggleVBox);
    btnBox->append(toggleGrp);

    mainTab->append("Buttons & Toggles", btnBox);

    // ==========================================
    // PAGE 2: Text Inputs & Sliders
    // ==========================================
    auto inputSplitter = SplitterView::create(SplitterOrientation::Horizontal)
        ->setSplitRatio(0.5f)
        ->setThickness(4.0f);

    auto leftInputBox = VBox::create()->setPadded(true);
    leftInputBox->append({
        {Label::create("Text Entries")->setFormat(LabelFormat::Normal), false},
        {Entry::create()->setHint("Standard single line...")->setWithContextMenu(true)->setUseContainerWidth(true), false},
        {PasswordEntry::create()->setHint("Password...")->setUseContainerWidth(true), false},
        {SearchEntry::create()->setHint("Search...")->setUseContainerWidth(true), false},
        {Entry::create("Multiline text area\nSupports multiple lines", false, true, 100.0f)->setUseContainerWidth(true), false}
    });

    auto rightInputBox = VBox::create()->setPadded(true);
    rightInputBox->append({
        {Label::create("Sliders & Spinners")->setFormat(LabelFormat::Normal), false},
        {SliderInt::create(0, 100)->setValue(50)->setUseContainerWidth(true), false},
        {SliderFloat::create(0.0f, 1.0f)->setValue(0.5f)->setLogarithmic(true)->setUseContainerWidth(true), false},
        {RangeSlider::create(0.0f, 100.0f)->setText("Min/Max")->setRange(20.0f, 80.0f)->setUseContainerWidth(true), false},
        {Spinbox::create(1, 10)->setValue(5), false},
        {ColorEdit::create(0.2f, 0.4f, 0.8f, 1.0f), false}
    });

    inputSplitter->setPanel1(leftInputBox);
    inputSplitter->setPanel2(rightInputBox);
    mainTab->append("Inputs & Sliders", inputSplitter);

    // ==========================================
    // PAGE 3: Data Presentation (Tables & Lists)
    // ==========================================
    auto dataVBox = VBox::create()->setPadded(true);

    dataVBox->append(BreadcrumbBar::create("Root/Documents/Images")->setIsEditing(false));

    auto comboHBox = HBox::create()->setPadded(true);
    auto cBox = ComboBox::create()->setPopupAlignLeft(true)->setMinWidth(200.0f);
    cBox->append("Item 1")->append("Item 2")->append("Item 3")->setSelectedIndex(0);
    comboHBox->append(cBox);
    dataVBox->append(comboHBox);

    auto listTableSplit = SplitterView::create(SplitterOrientation::Horizontal)->setSplitRatio(0.3f);

    auto lBox = ListBox::create()->setVisibleItems(10)->setSpanAvailWidth(true);
    for(int i=0; i<20; ++i) lBox->append("List Item " + std::to_string(i));
    listTableSplit->setPanel1(lBox);

    auto dataTable = Table::create()
        ->setSortable(true)
        ->setResizable(true)
        ->setReorderable(true)
        ->setSpanAvailWidth(true);

    dataTable->addColumn("ID", 0.5f, false)
             ->addColumn("Name", 2.0f, false)
             ->addColumn("Status", 1.0f, false);

    for(int i=0; i<15; ++i) {
        dataTable->addRow({
            Label::create(std::to_string(i)),
            Label::create("User " + std::to_string(i)),
            Checkbox::create("")->setChecked(i % 2 == 0)
        });
    }
    listTableSplit->setPanel2(dataTable);

    dataVBox->append({{listTableSplit, true}}); // Make stretchy
    mainTab->append("Data Views", dataVBox);

    // ==========================================
    // PAGE 4: Advanced Layouts & Shadows
    // ==========================================
    auto layoutGrid = Grid::create();

    auto propGrid = PropertyGrid::create()->setNameColumnWidth(150.0f);
    propGrid->addCategory("Transform", true)
            ->addProperty("Position X", SliderFloat::create(-100.f, 100.f))
            ->addProperty("Position Y", SliderFloat::create(-100.f, 100.f));
    propGrid->addCategory("Appearance", true)
            ->addProperty("Visible", ToggleSwitch::create(""));

    auto cardLayout = Card::create()->setPadding(10.0f)->setFillHeight(false);
    auto cardContent = VBox::create()->setPadded(true);
    cardContent->append({
        {Label::create("Shadow Card")->setFormat(LabelFormat::Wrapped), false},
        {ProgressBar::create()->setValue(0.75f)->setOverlayText("Loading 75%")->setSpanAvailWidth(true), false}
    });
    cardLayout->setChild(cardContent)
              ->setShadow(true, {2.0f, 2.0f}, 10.0f, {0.0f, 0.0f, 0.0f, 0.5f}, 5.0f);

    auto treeNode = TreeNode::create("Root Node")->setFramed(true);
    treeNode->append(TreeNode::create("Child Node A")->append(Label::create("Leaf Data")));
    treeNode->append(TreeNode::create("Child Node B")->setIconText("ICON_FOLDER"));

    layoutGrid->append(propGrid, 0, 0, 1);
    layoutGrid->append(cardLayout, 0, 1, 1);
    layoutGrid->append(treeNode, 1, 0, 2);
    layoutGrid->setColumnWeight(0, 1.0f)->setColumnWeight(1, 1.0f);

    mainTab->append("Layouts & Structures", layoutGrid);

    // Assign main layout to window
    mainWindow->setChild(mainTab);

    // 3. Application Main Loop
    App::run();
    App::shutdown();

    return 0;
} */