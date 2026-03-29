#include <mui.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>

// --- Forward Declarations ---

// UI Creation
mui::VBoxPtr createRootLayout(const mui::EntryPtr &themeNameEntry);
mui::HBoxPtr createToolbar(const mui::EntryPtr &themeNameEntry);
mui::SplitterViewPtr createMainSplitter();
mui::TabPtr createEditorPanel();
mui::VBoxPtr createColorsTab();
mui::VBoxPtr createGeometryTab();
mui::VBoxPtr createPreviewPanel();

// Helpers
std::string generateTomlTheme(const std::string &themeName);
std::string formatColor(const ImVec4 &c);
std::string formatVec2(const ImVec2 &v);

// --- Main Application ---

int main()
{
    // 1. Initialize the App
    mui::App::init(true); // Using OpenGL

    // By default, the App creates a dockspace that fills the main window.
    // We need to tell the App where to put our editor window.
    // This simple layout builder docks our main window into the dockspace.
    mui::App::setLayoutBuilder([](mui::DockBuilder &builder)
                               { builder.dockWindow("MUI Theme Editor", builder.getID()); });

    // 2. Create the Main Window
    auto mainWindow = mui::Window::create("MUI Theme Editor", 1000, 700)
                          ->setMargined(false); // We'll manage margins manually via layouts

    // Ensure the app quits when the window is closed.
    mainWindow->onClosing([]()
                          {
        mui::App::quit();
        return true; });

    // 3. Create the root layout and its state
    auto themeNameEntry = mui::Entry::create("MyCustomTheme")->setWidth(250.0f);
    auto rootLayout = createRootLayout(themeNameEntry);
    mainWindow->setChild(rootLayout);

    // 4. Show the window and run the app
    mainWindow->show();
    mui::App::run();

    return 0;
}

// --- UI Implementation ---

/**
 * @brief Creates the root layout for the window, containing the toolbar and main splitter.
 */
mui::VBoxPtr createRootLayout(const mui::EntryPtr &themeNameEntry)
{
    auto toolbar = createToolbar(themeNameEntry);
    auto mainSplitter = createMainSplitter();

    return mui::VBox::create()
        ->append(toolbar)
        ->append(mainSplitter, true); // True = Stretchy so it fills the rest of the window
}

/**
 * @brief Creates the top toolbar with theme name input and action buttons.
 */
mui::HBoxPtr createToolbar(const mui::EntryPtr &themeNameEntry)
{
    auto btnSave = mui::Button::create("Save Theme to TOML")
                       ->onClick([themeNameEntry]()
                                {
        mui::Dialogs::saveFile(
            "Save Theme",
            "TOML Files (*.toml){.toml},All Files (*.*){.*}",
            [themeNameEntry](const std::string &path)
            {
                std::string tomlContent = generateTomlTheme(themeNameEntry->getText());
                std::ofstream out(path);
                if (out.is_open())
                {
                    out << tomlContent;
                    out.close();
                    mui::Dialogs::msgBoxInfo("Success", "Theme saved successfully to:\n" + path);
                }
                else
                {
                    mui::Dialogs::msgBoxError("Error", "Failed to save theme file.");
                }
            }); });

    auto btnLoadLight = mui::Button::create("Load Light Default")
                            ->onClick([]()
                                       {
        mui::App::setTheme(mui::ThemeType::Light);
        // NOTE: This live-reloads the theme for the preview panel, but the
        // editor controls (sliders, color pickers) will not update to reflect
        // the new default values. This is a limitation of this simple tool.
    });

    auto btnLoadDark = mui::Button::create("Load Dark Default")
                           ->onClick([]()
                                      {
        mui::App::setTheme(mui::ThemeType::Dark);
        // NOTE: See 'Load Light Default' for limitations.
    });

    return mui::HBox::create()
        ->append(mui::Label::create("Theme Name:"))
        ->append(themeNameEntry)
        ->append(btnSave)
        // A vertical separator in an auto-sized HBox creates a layout paradox.
        // Use a fixed-size spacer instead.
        ->append(mui::Label::create("")->setWidth(15.f))
        ->append(btnLoadLight)
        ->append(btnLoadDark);
}

/**
 * @brief Creates the main horizontal splitter view between the editor and preview.
 */
mui::SplitterViewPtr createMainSplitter()
{
    auto editorPanel = createEditorPanel();
    auto previewPanel = createPreviewPanel();

    return mui::SplitterView::create(mui::SplitterOrientation::Horizontal)
        ->setSplitRatio(0.45f)
        ->setPanel1(editorPanel)
        ->setPanel2(previewPanel);
}

/**
 * @brief Creates the left-hand editor panel with tabs for Colors and Geometry.
 */
mui::TabPtr createEditorPanel()
{
    auto editorTabs = mui::Tab::create();
    editorTabs->append("Colors", createColorsTab());
    editorTabs->append("Geometry", createGeometryTab());
    return editorTabs;
}

/**
 * @brief Creates the content for the "Colors" tab in the editor.
 */
mui::VBoxPtr createColorsTab()
{
    auto colorGrid = mui::PropertyGrid::create()->setNameColumnWidth(160.0f);

    // Helper to add a color editor to the property grid and link it to a style color.
    auto addColorEditor = [&](mui::PropertyGridPtr grid, const std::string &label, ImGuiCol colIdx)
    {
        ImVec4 c = ImGui::GetStyle().Colors[colIdx];
        auto colorEdit = mui::ColorEdit::create(c.x, c.y, c.z, c.w);
        auto colorObservable = std::make_shared<mui::Observable<std::array<float, 4>>>(colorEdit->getColor());
        colorEdit->bind(colorObservable);
        grid->observe(colorObservable->onValueChanged, [colIdx](const std::array<float, 4> &newColor)
                      {
            // Direct live preview!
            ImGui::GetStyle().Colors[colIdx] = ImVec4(newColor[0], newColor[1], newColor[2], newColor[3]); });
        grid->addProperty(label, colorEdit);
    };

    colorGrid->addCategory("Base Backgrounds");
    addColorEditor(colorGrid, "Text", ImGuiCol_Text);
    addColorEditor(colorGrid, "Window Background", ImGuiCol_WindowBg);
    addColorEditor(colorGrid, "Child Background", ImGuiCol_ChildBg);
    addColorEditor(colorGrid, "Popup Background", ImGuiCol_PopupBg);
    addColorEditor(colorGrid, "Border", ImGuiCol_Border);

    colorGrid->addCategory("Interactive Widgets");
    addColorEditor(colorGrid, "Frame Background", ImGuiCol_FrameBg);
    addColorEditor(colorGrid, "Frame Hovered", ImGuiCol_FrameBgHovered);
    addColorEditor(colorGrid, "Frame Active", ImGuiCol_FrameBgActive);
    addColorEditor(colorGrid, "Button", ImGuiCol_Button);
    addColorEditor(colorGrid, "Button Hovered", ImGuiCol_ButtonHovered);
    addColorEditor(colorGrid, "Button Active", ImGuiCol_ButtonActive);
    addColorEditor(colorGrid, "CheckMark", ImGuiCol_CheckMark);
    addColorEditor(colorGrid, "Slider Grab", ImGuiCol_SliderGrab);
    addColorEditor(colorGrid, "Slider Grab Active", ImGuiCol_SliderGrabActive);

    colorGrid->addCategory("Layout & Navigation");
    addColorEditor(colorGrid, "Header", ImGuiCol_Header);
    addColorEditor(colorGrid, "Header Hovered", ImGuiCol_HeaderHovered);
    addColorEditor(colorGrid, "Header Active", ImGuiCol_HeaderActive);
    addColorEditor(colorGrid, "Tab", ImGuiCol_Tab);
    addColorEditor(colorGrid, "Tab Hovered", ImGuiCol_TabHovered);
    addColorEditor(colorGrid, "Tab Selected", ImGuiCol_TabSelected);
    addColorEditor(colorGrid, "Title Background", ImGuiCol_TitleBg);
    addColorEditor(colorGrid, "Title Active", ImGuiCol_TitleBgActive);
    addColorEditor(colorGrid, "Title Collapsed", ImGuiCol_TitleBgCollapsed);
    addColorEditor(colorGrid, "Separator", ImGuiCol_Separator);
    addColorEditor(colorGrid, "Separator Hovered", ImGuiCol_SeparatorHovered);
    addColorEditor(colorGrid, "Separator Active", ImGuiCol_SeparatorActive);

    return mui::VBox::create()->setScrollable(true)->append(colorGrid, true);
}

/**
 * @brief Creates the content for the "Geometry" tab in the editor.
 */
mui::VBoxPtr createGeometryTab()
{
    auto styleGrid = mui::PropertyGrid::create()->setNameColumnWidth(160.0f);
    ImGuiStyle &currentStyle = ImGui::GetStyle();

    // Helper to add a float slider to the property grid.
    auto addFloatEditor = [&](mui::PropertyGridPtr grid, const std::string &label, float *styleVar, float max = 24.0f)
    {
        auto slider = mui::SliderFloat::create(0.0f, max)->setValue(*styleVar);
        auto sliderObservable = std::make_shared<mui::Observable<float>>(*styleVar);
        slider->bind(sliderObservable);
        grid->observe(sliderObservable->onValueChanged, [styleVar](float val)
                      { *styleVar = val; });
        grid->addProperty(label, slider);
    };

    // Helper to add a dual-slider for ImVec2 to the property grid.
    auto addVec2Editor = [&](mui::PropertyGridPtr grid, const std::string &label, ImVec2 *styleVar, float max = 32.0f)
    {
        auto xSlider = mui::SliderFloat::create(0.0f, max)->setValue(styleVar->x);
        auto xObservable = std::make_shared<mui::Observable<float>>(styleVar->x);
        xSlider->bind(xObservable);
        grid->observe(xObservable->onValueChanged, [styleVar](float val)
                      { styleVar->x = val; });

        auto ySlider = mui::SliderFloat::create(0.0f, max)->setValue(styleVar->y);
        auto yObservable = std::make_shared<mui::Observable<float>>(styleVar->y);
        ySlider->bind(yObservable);
        grid->observe(
            yObservable->onValueChanged,
            [styleVar](float val)
            {
                styleVar->y = val;
            });
        auto box = mui::HBox::create()->append(xSlider, true)->append(ySlider, true);
        grid->addProperty(label, box);
    };

    styleGrid->addCategory("Rounding");
    addFloatEditor(styleGrid, "Window Rounding", &currentStyle.WindowRounding);
    addFloatEditor(styleGrid, "Frame Rounding", &currentStyle.FrameRounding);
    addFloatEditor(styleGrid, "Child Rounding", &currentStyle.ChildRounding);
    addFloatEditor(styleGrid, "Popup Rounding", &currentStyle.PopupRounding);
    addFloatEditor(styleGrid, "Tab Rounding", &currentStyle.TabRounding);
    addFloatEditor(styleGrid, "Grab Rounding", &currentStyle.GrabRounding);
    addFloatEditor(styleGrid, "Scrollbar Rounding", &currentStyle.ScrollbarRounding);

    styleGrid->addCategory("Borders");
    addFloatEditor(styleGrid, "Window Border", &currentStyle.WindowBorderSize, 4.0f);
    addFloatEditor(styleGrid, "Frame Border", &currentStyle.FrameBorderSize, 4.0f);
    addFloatEditor(styleGrid, "Popup Border", &currentStyle.PopupBorderSize, 4.0f);
    addFloatEditor(styleGrid, "Tab Border", &currentStyle.TabBorderSize, 4.0f);

    styleGrid->addCategory("Spacing & Padding");
    addVec2Editor(styleGrid, "Window Padding", &currentStyle.WindowPadding);
    addVec2Editor(styleGrid, "Frame Padding", &currentStyle.FramePadding);
    addVec2Editor(styleGrid, "Item Spacing", &currentStyle.ItemSpacing);
    addVec2Editor(styleGrid, "Item Inner", &currentStyle.ItemInnerSpacing);

    return mui::VBox::create()->setScrollable(true)->append(styleGrid, true);
}

/**
 * @brief Creates the right-hand preview panel with a sample of MUI widgets.
 */
mui::VBoxPtr createPreviewPanel()
{
    auto previewBox = mui::VBox::create()->setScrollable(true);
    previewBox->append(mui::Label::create("Live Theme Preview")->setFormat(mui::LabelFormat::Disabled));
    previewBox->append(mui::Separator::create());

    auto group1 = mui::Group::create("Standard Controls")->setMargined(true);
    auto group1Content = mui::VBox::create()
                             ->append(mui::Button::create("Primary Button"))
                             ->append(mui::Checkbox::create("Check me out"))
                             ->append(mui::ToggleSwitch::create("Enable Features"))
                             ->append(mui::Entry::create("Text input field..."))
                             ->append(mui::SliderFloat::create(0.0f, 100.0f)->setValue(50.0f))
                             ->append(mui::ProgressBar::create()->setValue(0.65f)->setOverlayText("65% Complete"));
    group1->setChild(group1Content);

    auto group2 = mui::Group::create("Lists & Selections")->setMargined(true);
    auto cbx = mui::ComboBox::create()->append("Option A")->append("Option B")->append("Option C")->setSelectedIndex(0);
    auto lbx = mui::ListBox::create()->append("Item 1")->append("Item 2")->append("Item 3")->setVisibleItems(3);
    auto group2Content = mui::VBox::create()
                             ->append(cbx)
                             ->append(lbx);
    group2->setChild(group2Content);

    auto cardPreview = mui::Card::create()->setPadding(16.0f)->setChild(
        mui::VBox::create()
            ->append(mui::Label::create("Card Surface"))
            ->append(mui::Separator::create()->setType(mui::SeparatorType::Text)->setText("Separator"))
            ->append(mui::Label::create("This is a card. It tests the ChildBg and ChildRounding properties.")->setWrapped(true)));

    previewBox->append(group1);
    previewBox->append(group2);
    previewBox->append(cardPreview);

    return previewBox;
}

// --- TOML Generation ---

/**
 * @brief Helper to format an ImVec4 as a TOML array string.
 * e.g., [1.000, 0.500, 0.250, 1.000]
 */
std::string formatColor(const ImVec4 &c)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "[" << c.x << ", " << c.y << ", " << c.z << ", " << c.w << "]";
    return ss.str();
}

/**
 * @brief Helper to format an ImVec2 as a TOML array string.
 * e.g., [8.000, 4.000]
 */
std::string formatVec2(const ImVec2 &v)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "[" << v.x << ", " << v.y << "]";
    return ss.str();
}

/**
 * @brief Generates a TOML string representing the current ImGui style.
 *
 * @param themeName The name to give the theme in the TOML file.
 * @return A string containing the theme data in TOML format.
 */
std::string generateTomlTheme(const std::string &themeName)
{
    ImGuiStyle &style = ImGui::GetStyle();
    std::stringstream ss;

    ss << "# Theme generated by MUI Theme Editor\n";
    ss << "[themes." << themeName << "]\n";

    // Output the active base inheritance
    std::string baseTheme = "Dark";
    if (mui::App::isUsingTomlTheme()) {
        baseTheme = mui::App::getCurrentThemeName();
    } else {
        baseTheme = (mui::App::getTheme() == mui::ThemeType::Light) ? "Light" : "Dark";
    }
    ss << "base = \"" << baseTheme << "\"\n";

    // --- STYLE SECTION ---
    ss << "# Style variables define the geometry and layout of widgets.\n";
    ss << "[themes." << themeName << ".style]\n";
    ss << std::fixed << std::setprecision(3);
    ss << "WindowRounding    = " << style.WindowRounding << "\n";
    ss << "ChildRounding     = " << style.ChildRounding << "\n";
    ss << "FrameRounding     = " << style.FrameRounding << "\n";
    ss << "PopupRounding     = " << style.PopupRounding << "\n";
    ss << "ScrollbarRounding = " << style.ScrollbarRounding << "\n";
    ss << "GrabRounding      = " << style.GrabRounding << "\n";
    ss << "TabRounding       = " << style.TabRounding << "\n\n";

    ss << "WindowBorderSize  = " << style.WindowBorderSize << "\n";
    ss << "ChildBorderSize   = " << style.ChildBorderSize << "\n";
    ss << "PopupBorderSize   = " << style.PopupBorderSize << "\n";
    ss << "FrameBorderSize   = " << style.FrameBorderSize << "\n";
    ss << "TabBorderSize     = " << style.TabBorderSize << "\n\n";

    ss << "WindowPadding     = " << formatVec2(style.WindowPadding) << "\n";
    ss << "FramePadding      = " << formatVec2(style.FramePadding) << "\n";
    ss << "ItemSpacing       = " << formatVec2(style.ItemSpacing) << "\n";
    ss << "ItemInnerSpacing  = " << formatVec2(style.ItemInnerSpacing) << "\n";

    // --- COLORS SECTION ---
    ss << "# Colors are RGBA arrays, with values from 0.0 to 1.0.\n";
    ss << "[themes." << themeName << ".colors]\n";

    auto dumpCol = [&](const std::string &key, ImGuiCol colIdx)
    {
        ss << std::left << std::setw(20) << key << " = " << formatColor(style.Colors[colIdx]) << "\n";
    };

    dumpCol("text", ImGuiCol_Text);
    dumpCol("textdisabled", ImGuiCol_TextDisabled);
    dumpCol("windowbg", ImGuiCol_WindowBg);
    dumpCol("childbg", ImGuiCol_ChildBg);
    dumpCol("popupbg", ImGuiCol_PopupBg);
    dumpCol("border", ImGuiCol_Border);
    dumpCol("framebg", ImGuiCol_FrameBg);
    dumpCol("framebghovered", ImGuiCol_FrameBgHovered);
    dumpCol("framebgactive", ImGuiCol_FrameBgActive);
    dumpCol("titlebg", ImGuiCol_TitleBg);
    dumpCol("titlebgactive", ImGuiCol_TitleBgActive);
    dumpCol("titlebgcollapsed", ImGuiCol_TitleBgCollapsed);
    dumpCol("button", ImGuiCol_Button);
    dumpCol("buttonhovered", ImGuiCol_ButtonHovered);
    dumpCol("buttonactive", ImGuiCol_ButtonActive);
    dumpCol("header", ImGuiCol_Header);
    dumpCol("headerhovered", ImGuiCol_HeaderHovered);
    dumpCol("headeractive", ImGuiCol_HeaderActive);
    dumpCol("tab", ImGuiCol_Tab);
    dumpCol("tabhovered", ImGuiCol_TabHovered);
    dumpCol("tabselected", ImGuiCol_TabSelected);
    dumpCol("separator", ImGuiCol_Separator);
    dumpCol("separatorhovered", ImGuiCol_SeparatorHovered);
    dumpCol("separatoractive", ImGuiCol_SeparatorActive);
    dumpCol("checkmark", ImGuiCol_CheckMark);
    dumpCol("slidergrab", ImGuiCol_SliderGrab);
    dumpCol("slidergrabactive", ImGuiCol_SliderGrabActive);

    return ss.str();
}