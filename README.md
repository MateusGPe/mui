This is the consolidated, exhaustive, and uncompromising source of truth for the MUI (Master UI) framework API and its internal architecture, documented comprehensively based solely on the provided source code.

-----

# 1\. Foundation and Core Architecture

MUI is a C++ graphical user interface framework that operates as a high-level, object-oriented, retained-mode wrapper around the immediate-mode ImGui library.

## 1.1 The Chainable Pattern

The core of the MUI framework relies on the `mui::Chainable<Derived>` template.

  * It provides a single public method: `std::shared_ptr<Derived> self()`.
  * This method performs a `static_cast` of the current instance down to the `Derived` type, invokes `shared_from_this()`, and then utilizes `std::static_pointer_cast` to return a `std::shared_ptr<Derived>`.
  * This mechanism guarantees that all widget configuration methods can be infinitely chained in a fluent builder pattern.

## 1.2 The IControl Base Interface

Every visual component in MUI derives from the non-templated `mui::IControl` interface to allow for polymorphism.

  * It maintains a `std::vector<mui::Connection> m_connections` which centralizes the connection management for all controls, ensuring safe lifecycles and automatic signal disconnection upon the destruction of the control.
  * It exposes pure virtual methods `virtual void render() = 0` and `virtual void onHandleDestroyed() = 0`.
  * It provides a `void clearConnections()` method to manually clear the vector of active signal connections.

## 1.3 The Control\<Derived\> Class

The `mui::Control<Derived>` template inherits from `mui::IControl` and `std::enable_shared_from_this<Derived>`. It encapsulates properties and methods universally available to all widgets.

### Universal Properties and State Management

  * **Visibility**: Managed via the `bool visible` property. Manipulated using `std::shared_ptr<Derived> show()` to set it to true, and `std::shared_ptr<Derived> hide()` to set it to false.
  * **Enablement**: Managed via the `bool enabled` property. Manipulated using `std::shared_ptr<Derived> setEnabled(bool enabled)` and queried via `bool isEnabled() const`.
  * **Handle Ownership**: Managed via `bool ownsHandle`. Altered using `std::shared_ptr<Derived> releaseOwnership()` and `std::shared_ptr<Derived> acquireOwnership()`. The base implementation of `onHandleDestroyed()` sets `ownsHandle` to false.
  * **Tooltip**: Managed via `std::string tooltip`. Set using `std::shared_ptr<Derived> setTooltip(const std::string &t)`. Rendered internally via `renderTooltip()` if the item is hovered with `ImGuiHoveredFlags_DelayNormal`.
  * **Stable Identity**: Managed via `std::string m_customId`. Modified using `std::shared_ptr<Derived> setID(const std::string& id)` and queried via `std::string getID() const`. Used by `pushControlID()` and `popControlID()` to prevent ImGui state loss upon reallocation.

### Universal Sizing and Positioning

  * **Absolute Sizing**: Managed via `float width` and `float height`.
      * Setters: `std::shared_ptr<Derived> setWidth(float w)`, `std::shared_ptr<Derived> setHeight(float h)`, and `std::shared_ptr<Derived> setSize(float w, float h)`.
      * Getters: `float getWidth() const`, `float getHeight() const`, and `ImVec2 getSize() const`.
      * Modifiers: `std::shared_ptr<Derived> addSize(float w, float h)`, `std::shared_ptr<Derived> scaleSize(float factor)`, and `std::shared_ptr<Derived> scaleSize(float factorX, float factorY)`.
  * **Layout Constraints**: Managed via `ImVec2 minSize` and `ImVec2 maxSize`.
      * Setters: `std::shared_ptr<Derived> setMinSize(float w, float h)`, `std::shared_ptr<Derived> setMaxSize(float w, float h)`, `std::shared_ptr<Derived> setMinWidth(float w)`, `std::shared_ptr<Derived> setMinHeight(float h)`, `std::shared_ptr<Derived> setMaxWidth(float w)`, and `std::shared_ptr<Derived> setMaxHeight(float h)`.
      * Getters: `ImVec2 getMinSize() const` and `ImVec2 getMaxSize() const`.
  * **Absolute Positioning**: Managed via `ImVec2 position` initialized to `{-1.0f, -1.0f}`.
      * Setters: `std::shared_ptr<Derived> setPosition(float x, float y)`, `std::shared_ptr<Derived> setX(float x)`, and `std::shared_ptr<Derived> setY(float y)`.
      * Getters: `ImVec2 getPosition() const`, `float getX() const`, and `float getY() const`.
      * Modifiers: `std::shared_ptr<Derived> addPosition(float x, float y)` (starts from 0,0 if currently auto-positioning).
  * **Dynamic Width Policies**:
      * `std::shared_ptr<Derived> setSpanAvailWidth(bool span)` sets the `spanAvailWidth` boolean.
      * `std::shared_ptr<Derived> setUseContainerWidth(bool use)` sets the `useContainerWidth` boolean.

### Universal Shadow System

  * **Global Shadow Defaults**: Managed via static members `s_defaultHasShadow`, `s_defaultShadowOffset`, `s_defaultShadowBlur`, `s_defaultShadowColor`, and `s_defaultShadowRounding`. These are configured utilizing `static void setGlobalShadowDefaults(bool enable, ImVec2 offset, float blur, ImVec4 col, float rounding)`.
  * **Instance Shadow Properties**: Managed via `hasShadow`, `shadowOffset`, `shadowBlur`, `shadowColor`, and `shadowRounding`.
  * **Configuration**: `std::shared_ptr<Derived> setShadow(bool enable, ImVec2 offset, float blur, ImVec4 col, float rounding)` overrides instance shadows, and `std::shared_ptr<Derived> defaultShadow(bool enable)` resets the instance to use global defaults.
  * **Shadow Rendering**: The final overridden `render()` method splits ImGui draw channels, executes the pure virtual `virtual void renderControl() = 0` in the foreground to obtain dimensions, draws the shadow in the background using `AddRectFilled` with simulated soft falloff iterations, and finally merges the channels.

### Signal Observation

  * `std::shared_ptr<Derived> observe(Signal<Args...> &signal, F cb)` connects a callback to a provided signal and pushes the connection to `m_connections`.

-----

# 2\. Application and Window Management

## 2.1 mui::App

The `mui::App` class is the central orchestrator for the lifecycle, threading, layout, and rendering backend.

  * **Initialization**: `static void init(bool useOpenGL = false)` initializes SDL3 video and events, creates an SDL Window configured for high pixel density, creates either an OpenGL context or SDL Renderer, initiates ImGui with docking enabled, applies the current theme, and creates texture mapping callbacks for the file dialog system.
  * **Execution**: `static void run()` executes the main blocking event loop, polling SDL events, tracking DPI changes, draining the thread-safe UI task queue, rendering the ImGui frame, updating docking nodes, processing message boxes, and dispatching the user's main loop callback.
  * **Termination**: `static void quit()` breaks the main loop, and `static void shutdown()` concludes the process.
  * **Event Hooks**: `static void setMainLoopCallback(std::function<void()> cb)` sets a callback executed every frame.
  * **Layout Topology**: `static void setLayoutBuilder(std::function<void(DockBuilder &)> cb)` provides a builder block called exactly once upon initial layout initialization to map windows to dock nodes.
  * **Thread Safety**: `static void assertMainThread()` throws a `std::runtime_error` if executed outside the thread ID captured during `init()`. `static void queueMain(std::function<void()> callback)` pushes a closure to a `std::mutex` locked `std::queue` which is drained by the main loop prior to frame rendering via `drainMainQueue()`.
  * **Dialog Dispatching**: `static void addMessageBox(ActiveMessageBox &&mb)` pushes a messagebox onto the sequential popup queue. `static void processMessageBoxes()` resolves the queue frame-by-frame.
  * **State Getters**: `static SDL_GLContext getGLContext()`, `static std::string getFilepath()`, `static float getDpiScale()`.
  * **Theme Management**:
      * `static void setTheme(ThemeType type)` updates the theme to `ThemeType::Light` or `ThemeType::Dark`.
      * `static void setTheme(const std::string &filepath, const std::string &themeName)` and `static void setTheme(const std::string &themeName)` load configuration from a TOML file.
      * `static std::vector<std::string> getAvailableThemes()` parses `themes.toml` to return a list of theme names.
      * `static void requestDpiUpdate()` flags the engine to rebuild fonts and styles on the next frame.
  * **Color Filters**: The App supports global frame-buffer level color adjustments triggered via `static void setApplyGrayscale(bool)`, `static void setApplyComplementary(bool)`, `static void setApplySepia(bool)`, and `static void setApplyInvert(bool)`. Companion getters exist (e.g., `getApplyGrayscale()`), alongside a `static void resetColorModifiers()` method.

## 2.2 mui::Window

A top-level, floating, dockable ImGui window inheriting from `Control<Window>`.

  * **Creation**: `static WindowPtr create(const std::string &title, int width, int height, bool hasMenubar = false)`. The constructor registers the instance into `App::activeWindows`.
  * **Core Configuration**:
      * `WindowPtr setChild(IControlPtr child)` assigns the root widget node.
      * `WindowPtr setTitle(const std::string &title)` and `std::string getTitle() const` manage the window identifier string.
      * `WindowPtr setContentSize(int width, int height)` and `std::pair<int, int> getContentSize() const` dictate the first-use physical bounds.
      * `WindowPtr setMargined(bool margined)` and `bool getMargined() const` toggle the application of `ImGuiStyleVar_WindowPadding`.
      * `WindowPtr setDockId(Identifier id)` enforces a target ImGui dock space node.
      * `WindowPtr focus()` schedules `ImGui::SetNextWindowFocus()` for the next frame.
      * `WindowPtr setNoTabBar(bool b)` and `bool getNoTabBar() const` toggle `ImGuiDockNodeFlags_NoTabBar` on the host window class.
  * **Lifecycle**:
      * `WindowPtr onClosing(std::function<bool()> cb)` registers a callback executed when the user attempts to close the window. The window remains open if the callback returns `false`.
      * `void close()` programmatically triggers closure.
      * `bool isWindowOpen() const` checks current state.
  * **ImGuiWindowFlags Configuration**: The `Window` class provides extensive boolean toggles mapping directly to underlying ImGui bit flags. These include:
      * `setBorderless(bool)` / `getBorderless()` (Toggles `ImGuiWindowFlags_NoDecoration`).
      * `setHasMenubar(bool)` / `getHasMenubar()`.
      * `setResizable(bool)` / `getResizable()`.
      * `setMovable(bool)` / `getMovable()`.
      * `setCollapsible(bool)` / `getCollapsible()`.
      * `setHasTitlebar(bool)` / `getHasTitlebar()`.
      * `setScrollbar(bool)` / `getScrollbar()`.
      * `setScrollWithMouse(bool)` / `getScrollWithMouse()`.
      * `setAlwaysAutoResize(bool)` / `getAlwaysAutoResize()`.
      * `setBackground(bool)` / `getBackground()`.
      * `setSavedSettings(bool)` / `getSavedSettings()`.
      * `setMouseInputs(bool)` / `getMouseInputs()`.
      * `setHorizontalScrollbar(bool)` / `getHorizontalScrollbar()`.
      * `setFocusOnAppearing(bool)` / `getFocusOnAppearing()`.
      * `setBringToFrontOnFocus(bool)` / `getBringToFrontOnFocus()`.
      * `setAlwaysVerticalScrollbar(bool)` / `getAlwaysVerticalScrollbar()`.
      * `setAlwaysHorizontalScrollbar(bool)` / `getAlwaysHorizontalScrollbar()`.
      * `setNavInputs(bool)` / `getNavInputs()`.
      * `setNavFocus(bool)` / `getNavFocus()`.
      * `setUnsavedDocument(bool)` / `getUnsavedDocument()`.
      * `setDocking(bool)` / `getDocking()`.

## 2.3 Docking Definitions

MUI encapsulates ImGui's docking node manipulations via `mui::DockBuilder`, `mui::DockNode`, and `mui::DockDirection`.

  * **`DockDirection`**: An enum class mapping to `ImGuiDir_Left`, `ImGuiDir_Right`, `ImGuiDir_Up`, `ImGuiDir_Down`.
  * **`DockNode`**: Wraps `ImGuiDockNode*`. Provides `void setNoTabBar(bool noTabBar)`, `void setHiddenTabBar(bool hiddenTabBar)`, and `bool isValid() const`.
  * **`DockBuilder`**: Instantiated with an `ImGuiID dockspaceId`.
      * `ImGuiID getID() const` returns the parent space ID.
      * `ImGuiID splitNode(ImGuiID &nodeIdToSplit, DockDirection dir, float ratio)` delegates to `ImGui::DockBuilderSplitNode` and yields a new node ID.
      * `DockNode getNode(ImGuiID nodeId)` accesses the underlying node object.
      * `void dockWindow(const char *windowName, ImGuiID nodeId)` and `void dockWindow(const std::string &windowName, ImGuiID nodeId)` instruct the builder to slot a named window into a designated node ID.

-----

# 3\. Signals and Observables (State Management)

## 3.1 mui::Signal

A thread-safe, generic event emitter implementing connection lifecycle tracking.

  * **State**: Maintains a `std::vector<Slot> m_slots` where a `Slot` is a struct containing a `std::weak_ptr<bool> alive` and a `std::function<void(Args...)> cb`, guarded by a `std::mutex`.
  * **Methods**:
      * `Connection connect(std::function<void(Args...)> cb)` allocates a `std::shared_ptr<bool>` control block, pushes the slot, and returns a `Connection`.
      * `void operator()(Args... args)` invokes the signal. It safely cleans up dead connections holding expired weak pointers, locks to copy active callbacks into a local vector, and executes them outside the lock to permit re-entrancy without deadlocking.
      * `size_t slot_count() const` returns the number of active, non-expired connections.
      * `void clear()` indiscriminately wipes all slots.

## 3.2 mui::Connection

An opaque token binding a callback to a `Signal`. The connection remains valid as long as this object (or copies of it) remains in scope.

  * **State**: Holds `std::shared_ptr<bool> m_alive`.
  * **Methods**: `void disconnect()` explicitly resets the pointer, and `bool connected() const` evaluates pointer validity.

## 3.3 mui::Observable\<T\>

A thread-safe reactive data variable.

  * **State**: Holds a value of type `T` and an internal `std::mutex`.
  * **Signals**: Exposes `mui::Signal<const T&> onValueChanged`.
  * **Methods**:
      * `void set(const T& new_val)` acquires the lock, overwrites the value if altered, and invokes `onValueChanged` outside the lock context.
      * `T get() const` performs a thread-safe read.
      * The `=` assignment operator is overloaded to trigger `set(new_val)`, and a casting operator `operator T() const` routes to `get()`.

-----

# 4\. Scoped ImGui Utilities

MUI encapsulates ImGui's stack-based state parameters into RAII-compliant scoped classes.

  * **`ScopedColor`**: Pushes an `ImGuiCol` index alongside either an `ImU32` or an `ImVec4` color. Pops automatically on destruction.
  * **`ScopedStyle`**: Pushes an `ImGuiStyleVar` index alongside either a `float` or an `ImVec2`. Pops automatically on destruction.
  * **`ScopedID`**: Wraps `ImGui::PushID` and `ImGui::PopID`, accepting `int`, `const char*`, `const void*`, or `std::string` constructors.
  * **`ScopedItemWidth`**: Pushes a `float item_width` via `ImGui::PushItemWidth` and pops upon destruction.

-----

# 5\. Core Layout Containers

## 5.1 Box Layouts (VBox, HBox, FlowBox)

All box layouts inherit from `Box<Derived>`, which defines a `struct BoxChild` containing `IControlPtr control` and `bool stretchy`.

  * **Box\<Derived\> Methods**:
      * `std::shared_ptr<Derived> append(IControlPtr child, bool stretchy = false)` and `std::shared_ptr<Derived> append(std::initializer_list<BoxChild> items)` push widgets into the layout block.
      * `std::shared_ptr<Derived> deleteChild(int index)` erases a child at a designated index.
      * `std::shared_ptr<Derived> setPadded(bool p)` and `bool getPadded() const` dictate spacing insertion.
      * `std::shared_ptr<Derived> setScrollable(bool s)` and `bool getScrollable() const` toggle container scrolling flags.
      * `std::shared_ptr<Derived> setAutoScroll(bool a)` and `bool getAutoScroll() const` control forced scrolling to the maximum Y boundary upon rendering.
      * `int getNumChildren() const` returns the child vector magnitude.
  * **`VBox`**: Renders children vertically utilizing an internal 1-column ImGui table configured with `ImGuiTableFlags_SizingStretchSame`. Stretchy children receive `-1` item widths.
  * **`HBox`**: Renders children horizontally utilizing an N-column ImGui table. Stretchy children dictate column configuration as `ImGuiTableColumnFlags_WidthStretch` over `ImGuiTableColumnFlags_WidthFixed`, and receive `-FLT_MIN` item widths.
  * **`FlowBox`**: Arranges children inline and calculates bounding boxes (`lastKnownWidth`) to conditionally wrap them onto new lines based on `ImGui::GetContentRegionAvail().x`.
      * **Alignment**: Configured via `FlowBoxPtr setAlign(Align align)` utilizing the enum `FlowBox::Align` encompassing `Left`, `Center`, `Right`, and `Justify`.

## 5.2 Grid Layout

The `Grid` widget provides a rigid matrix layout.

  * **Creation**: `static GridPtr create()`.
  * **Configuration**:
      * `GridPtr append(IControlPtr child, int row, int col, int colSpan = 1)` maps a widget to explicit coordinates, automatically updating maximum row and column thresholds.
      * `GridPtr setColumnWeight(int col, float weight)` registers proportional sizing logic utilizing `ImGuiTableColumnFlags_WidthStretch`.

## 5.3 Group

A collapsible structural container wrapping `ImGui::CollapsingHeader`.

  * **Creation**: `static GroupPtr create(const std::string &title)`.
  * **Configuration**:
      * `GroupPtr setChild(IControlPtr c)` dictates the payload rendered when open.
      * `GroupPtr setMargined(bool margined)` and `bool getMargined() const` wrap child rendering in `ImGui::Indent()` and `ImGui::Unindent()`.
      * `GroupPtr setTitle(const std::string &title)` and `std::string getTitle() const` mutate the header string.
      * `GroupPtr setOpen(bool open)` and `bool isOpen() const` alter the collapsible state.
  * **Binding & Events**:
      * `GroupPtr onToggled(std::function<void(bool)> cb)` provides a hook for state mutations.
      * `GroupPtr bindTitle(std::shared_ptr<Observable<std::string>> observable)`.
      * `GroupPtr bindOpen(std::shared_ptr<Observable<bool>> observable)` provides two-way boolean binding.
  * **ImGuiTreeNodeFlags Settings**: The group encapsulates toggles modifying the header appearance and behavior:
      * `defaultOpen(bool)`, `setAllowOverlap(bool)`, `setOpenOnDoubleClick(bool)`, `setOpenOnArrow(bool)`, `setBullet(bool)`, `setFramePadding(bool)`, `setSpanAvailWidth(bool)`, `setSpanFullWidth(bool)`, `setNavLeftJumpsToParent(bool)`, `setFramed(bool)`, `setSelected(bool)`, `setNoTreePushOnOpen(bool)`, `setNoAutoOpenOnLog(bool)`, `setLeaf(bool)`, `setSpanAllColumns(bool)`.

## 5.4 TreeNode

A recursive collapsible hierarchy node.

  * **Creation**: `static TreeNodePtr create(const std::string& label)`.
  * **Configuration**:
      * `TreeNodePtr append(IControlPtr child)` mounts a sub-node or standard control.
      * `TreeNodePtr setIconText(const std::string& t)` prepends string-based icons to the text.
      * `TreeNodePtr setIconTexture(ImTextureID tex)` renders an image texture prior to the label text via manual draw list positioning.
  * **Events**: `TreeNodePtr onClick(std::function<void()> cb)` and `TreeNodePtr onDoubleClick(std::function<void()> cb)`.
  * **Binding**: `bool isSelected() const` and `TreeNodePtr bindSelected(std::shared_ptr<Observable<bool>> observable)` manage selection.
  * **ImGuiTreeNodeFlags Settings**: Mirrors the exhaustive flag mapping provided by the `Group` control.

## 5.5 Tab

A tabbed multipage container.

  * **Creation**: `static TabPtr create()`.
  * **Configuration**:
      * `TabPtr append(const std::string &name, IControlPtr child)` generates a new page.
      * `TabPtr setMargined(int page, bool margined)` overrides child rendering to encapsulate it within a padded `ImGuiChildFlags_FrameStyle` child window.
      * `TabPtr setSelected(int index)`, `int getSelected() const`, and `int getNumPages() const` manage page index navigation.
  * **Binding & Events**: `TabPtr onSelected(std::function<void(int)> cb)` and `TabPtr bindSelected(std::shared_ptr<Observable<int>> observable)`.
  * **ImGuiTabBarFlags Settings**: Controls global tab behavior:
      * `setReorderable(bool)`, `setAutoSelectNewTabs(bool)`, `setNoCloseWithMiddleMouseButton(bool)`, `setNoTabListScrollingButtons(bool)`, `setNoTooltip(bool)`, `setFittingPolicyScroll(bool)`, `setFittingPolicyResizeDown(bool)`.

## 5.6 SplitterView

A paned layout bridged by an interactive split handle.

  * **Creation**: `static SplitterViewPtr create(SplitterOrientation o = SplitterOrientation::Horizontal)` where `SplitterOrientation` encompasses `Horizontal` and `Vertical`.
  * **Configuration**:
      * `SplitterViewPtr setPanel1(IControlPtr c)` and `SplitterViewPtr setPanel2(IControlPtr c)` define the content.
      * `SplitterViewPtr setSplitRatio(float ratio)` and `float getSplitRatio() const` manage the proportional distribution (clamped strictly between 0.05f and 0.95f).
      * `SplitterViewPtr setThickness(float t)` defines the interaction area size of the splitter button.
  * **Binding & Events**: `SplitterViewPtr onSplitRatioChanged(std::function<void(float)> cb)` and `SplitterViewPtr bindSplitRatio(std::shared_ptr<Observable<float>> observable)`.

-----

# 6\. Detailed Widget Implementations

## 6.1 Button

  * **Enum `ButtonType`**: `Normal`, `Small`, `Invisible`, `ArrowLeft`, `ArrowRight`.
  * **Creation**: `static ButtonPtr create(const std::string &text)`.
  * **Configuration**:
      * `std::string getText() const` and `ButtonPtr setText(const std::string &t)`.
      * `ButtonPtr setType(ButtonType t)` executes specialized ImGui rendering implementations (e.g., `ImGui::SmallButton`, `ImGui::ArrowButton`).
      * `ButtonPtr setColor(ImVec4 c, ImVec4 hover, ImVec4 active)` overrides native theming via `ScopedColor`.
  * **Events**: `ButtonPtr onClick(std::function<void()> cb)` connects to `mui::Signal<> onClickSignal`.

## 6.2 IconButton

A composite button integrating text alongside standard or texture-based icons.

  * **Enum `IconButtonLayout`**: `Horizontal`, `Vertical`.
  * **Creation**: `static IconButtonPtr create(const std::string &text = "")`.
  * **Configuration**:
      * `IconButtonPtr setText(const std::string &t)`.
      * `IconButtonPtr setIconText(const std::string &t)` and `IconButtonPtr setIconTexture(ImTextureID tex)` dictate the visual icon mode.
      * `IconButtonPtr setLayout(IconButtonLayout l)` alters draw-list rendering geometry.
      * `IconButtonPtr setSelected(bool s)` applies `ImGuiCol_Header` highlight colors simulating a toggled state.
      * `IconButtonPtr setIconSize(float w, float h)` establishes dimensions.
      * `IconButtonPtr setUseContainerWidth(bool use)` defers width logic.
  * **Events**: `IconButtonPtr onClick(std::function<void()> cb)` and `IconButtonPtr onDoubleClick(std::function<void()> cb)` connect to `onClickSignal` and `onDoubleClickSignal` respectively.

## 6.3 IconStack

Generates an inline cluster of purely graphical buttons sharing uniform dimensions.

  * **Creation**: `static IconStackPtr create()`.
  * **Configuration**: `IconStackPtr add(const std::string &icon, std::function<void()> onClick, const std::string &tip = "", bool textHover = false)` pushes a struct defining the button, linking the specific click index to `onIconClickedSignal`.

## 6.4 Checkbox

  * **Creation**: `static CheckboxPtr create(const std::string &text)`.
  * **Configuration**:
      * `bool isChecked() const` and `CheckboxPtr setChecked(bool c)` manipulate binary state.
      * `std::string getText() const` and `CheckboxPtr setText(const std::string &t)`.
      * `CheckboxPtr setScale(float s)` manipulates mathematical rendering dimensions.
  * **Binding & Events**: `CheckboxPtr bind(std::shared_ptr<Observable<bool>> observable)` and `CheckboxPtr onToggled(std::function<void(bool)> cb)` hooking into `mui::Signal<bool> onToggledSignal`.

## 6.5 ToggleSwitch

A customized binary switch mimicking modern mobile OS interaction.

  * **Creation**: `static ToggleSwitchPtr create(const std::string &label)`.
  * **Configuration**: `bool isChecked() const`, `ToggleSwitchPtr setChecked(bool c)`, and `ToggleSwitchPtr setScale(float s)`.
  * **Binding & Events**: `ToggleSwitchPtr bind(std::shared_ptr<Observable<bool>> observable)` and `ToggleSwitchPtr onToggled(std::function<void(bool)> cb)`.

## 6.6 RadioButton

  * **Creation**: `static RadioButtonPtr create(const std::string &text)`.
  * **Configuration**: `bool isChecked() const` and `RadioButtonPtr setChecked(bool c)`.
  * **Grouping Mechanism**: `static void group(std::initializer_list<RadioButtonPtr> buttons)` instantiates a shared integer pointer resolving mutual exclusivity across the provided list of buttons.
  * **Binding & Events**: `RadioButtonPtr bind(std::shared_ptr<Observable<bool>> observable)` and `RadioButtonPtr onToggled(std::function<void(bool)> cb)`.

## 6.7 Entry Family (Text Input)

Comprises `Entry`, `PasswordEntry`, and `SearchEntry` classes.

  * **Creation**: `static EntryPtr create(const std::string &initialText = "", bool password = false, bool multiline = false, float h = 0.0f)` or `static EntryPtr create(char *buf, size_t buf_size)` for raw buffer manipulation. `PasswordEntry::create()` and `SearchEntry::create()` encapsulate specific pre-configurations.
  * **Configuration**:
      * `std::string getText() const` and `EntryPtr setText(const std::string &t)`.
      * `EntryPtr setHint(const std::string &h)` establishes grayed placeholder text.
      * `EntryPtr setMultiline(bool m, float h = 0.0f)` swaps the rendering mechanism to `ImGui::InputTextMultiline`.
      * `EntryPtr setWidth(float w)` and `EntryPtr setUseContainerWidth(bool use)` modify spatial logic.
  * **ImGuiInputTextFlags Settings**: `setReadOnly(bool)`, `setAutoSelectAll(bool)`, `setNoSpaces(bool)`, `setCharsDecimal(bool)`, `setCharsHexadecimal(bool)`, `setCharsUppercase(bool)`, `setAllowTabInput(bool)`, `setCtrlEnterForNewLine(bool)`.
  * **Context Menu**: `EntryPtr setWithContextMenu(bool c)` dictates the generation of an inline context popup for Copy, Cut, Paste, and Clear functionality upon a right-click.
  * **Binding & Events**: `EntryPtr bind(std::shared_ptr<Observable<std::string>> observable)`, `EntryPtr onChanged(std::function<void()> cb)`, and `EntryPtr onEnter(std::function<void(const std::string &)> cb)`.

## 6.8 Numerical Sliders (SliderInt & SliderFloat)

  * **Creation**: `static SliderIntPtr create(int min, int max)` and `static SliderFloatPtr create(float min, float max)`.
  * **Configuration**: `getValue() const`, `setValue(value)`, `setFormat(const std::string &f)`, and `setLogarithmic(bool l)` (toggles `ImGuiSliderFlags_Logarithmic`).
  * **Binding & Events**: `bind(std::shared_ptr<Observable<T>> observable)` and `onChanged(std::function<void()> cb)` mapped to `mui::Signal<T> onChangedSignal`.

## 6.9 RangeSlider

A custom component implementing two grab handles for bounded ranges.

  * **Creation**: `static RangeSliderPtr create(float min, float max)`.
  * **Configuration**:
      * `std::string getText() const` and `RangeSliderPtr setText(const std::string &t)` establish the label.
      * `std::pair<float, float> getRange() const` and `RangeSliderPtr setRange(float vMin, float vMax)` alter bounds.
      * `RangeSliderPtr setFormat(const std::string &f)` alters tooltip and overlay formatting.
      * `RangeSliderPtr setUseContainerWidth(bool use)` manipulates layout behavior.
  * **Binding & Events**: `RangeSliderPtr bind(std::shared_ptr<Observable<std::pair<float, float>>> observable)` and `RangeSliderPtr onChanged(std::function<void(float, float)> cb)`.

## 6.10 Spinbox

  * **Creation**: `static SpinboxPtr create(int min, int max)`.
  * **Configuration**: `int getValue() const` and `SpinboxPtr setValue(int v)` clamped strictly by constructor minimum and maximum values.
  * **Binding & Events**: `SpinboxPtr bind(std::shared_ptr<Observable<int>> observable)` and `SpinboxPtr onChanged(std::function<void()> cb)`.

## 6.11 ComboBox

  * **Creation**: `static ComboBoxPtr create()`.
  * **Configuration**:
      * `ComboBoxPtr append(const std::string &item)` builds the dropdown list.
      * `ComboBoxPtr clear()` resets the content vector and selection.
      * `int getSelectedIndex() const` and `ComboBoxPtr setSelectedIndex(int index)` target the active integer.
      * `std::string getText() const` extracts the associated string array element.
      * `ComboBoxPtr setSpanAvailWidth(bool span)` and `ComboBoxPtr setMinWidth(float w)` alter footprint.
  * **ImGuiComboFlags Settings**: `setPopupAlignLeft(bool)`, `setHeightSmall(bool)`, `setHeightRegular(bool)`, `setHeightLarge(bool)`, `setHeightLargest(bool)`, `setNoArrowButton(bool)`, `setNoPreview(bool)`, `setCustomPreview(bool)`.
  * **Binding & Events**: `ComboBoxPtr bind(std::shared_ptr<Observable<int>> observable)` and `ComboBoxPtr onChanged(std::function<void(int)> cb)`.

## 6.12 ListBox

  * **Creation**: `static ListBoxPtr create()`.
  * **Configuration**:
      * `ListBoxPtr append(const std::string &item)` and `ListBoxPtr clear()` structure content.
      * `int getSelected() const` and `ListBoxPtr setSelected(int index)` handle internal pointers.
      * `ListBoxPtr setVisibleItems(int count)` adjusts vertical height multiplier.
      * `ListBoxPtr setSpanAvailWidth(bool span)` and `ListBoxPtr setUseContainerWidth(bool use)`.
  * **Binding & Events**: `ListBoxPtr bind(std::shared_ptr<Observable<int>> observable)`, `ListBoxPtr onSelected(std::function<void()> cb)`, and `ListBoxPtr onDoubleClick(std::function<void()> cb)` linking to `onSelectedSignal` and `onDoubleClickSignal` respectively.

## 6.13 BreadcrumbBar

A segmented navigator allowing deep-path drilling with text-editing capabilities.

  * **Creation**: `static BreadcrumbBarPtr create(const std::string &path = "")`.
  * **Configuration**:
      * `std::string getPath() const` and `BreadcrumbBarPtr setPath(const std::string &path)`.
      * `bool getIsEditing() const` and `BreadcrumbBarPtr setIsEditing(bool editing)` toggles a free-type `mui::Entry` block taking over the child layout area.
  * **Binding & Events**: `BreadcrumbBarPtr bind(std::shared_ptr<Observable<std::string>> observable)` and `BreadcrumbBarPtr onPathNavigated(std::function<void(const std::string &)> cb)`.

## 6.14 Table

An implementation wrapping `ImGui::BeginTable` and sorting specifications.

  * **Creation**: `static TablePtr create()`.
  * **Configuration**:
      * `TablePtr addColumn(const std::string &name, float weight = 1.0f, bool fixedWidth = false)` dictates the `TableColumn` definitions.
      * `TablePtr addRow(const std::vector<IControlPtr> &rowItems)` pushes a linear vector mapping elements into instantiated columns.
      * `TablePtr clearRows()` empties all vectors.
      * `int getSelectedRow() const` and `TablePtr setSelectedRow(int index)`.
  * **ImGuiTableFlags Settings**: `setSortable(bool)`, `setResizable(bool)`, `setReorderable(bool)`, `setHideable(bool)`, `setNoSavedSettings(bool)`, `setContextMenuInBody(bool)`, `setScrollX(bool)`.
  * **Binding & Events**: `TablePtr bind(std::shared_ptr<Observable<int>> observable)`, `TablePtr onRowSelected(std::function<void(int)> cb)`, `TablePtr onRowDoubleClicked(std::function<void(int)> cb)`, and `TablePtr onSortRequested(std::function<void(int, bool)> cb)` (emitting column index and ascending boolean).

## 6.15 PropertyGrid

A standardized dual-column layout tailored for category-header grouped property editors.

  * **Creation**: `static PropertyGridPtr create()`.
  * **Configuration**:
      * `PropertyGridPtr addCategory(const std::string &name, bool defaultOpen = true)` inserts a logical partition mapped to a `TreeNode`.
      * `PropertyGridPtr addProperty(const std::string &label, IControlPtr editor)` mounts a widget on the active category.
      * `PropertyGridPtr setNameColumnWidth(float width)` enforces a fixed dimension for the label column.

## 6.16 ColorEdit

  * **Creation**: `static ColorEditPtr create(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)`.
  * **Configuration**: `std::array<float, 4> getColor() const` and `ColorEditPtr setColor(float r, float g, float b, float a = 1.0f)` operating on an internal array.
  * **Binding & Events**: `ColorEditPtr bind(std::shared_ptr<Observable<std::array<float, 4>>> observable)` and `ColorEditPtr onChanged(std::function<void(const std::array<float, 4> &)> cb)`.

## 6.17 Presentation Widgets

  * **`Label`**:
      * Creation: `static LabelPtr create(const std::string &text)`.
      * Configuration: `getText() const`, `setText(t)`, `setFormat(LabelFormat f)` utilizing `LabelFormat::Normal`, `Wrapped`, `Bullet`, `Disabled`. `setWrapped(bool w)` serves as a convenience alias. `setColor(ImVec4 c)` overrides global text styling.
  * **`Image`**:
      * Creation: `static ImagePtr create(ImTextureID tex, float w, float h)`.
      * Configuration: `setTexture(ImTextureID tex)`, `setUV(float u0, float v0, float u1, float v1)`, `setTint(ImVec4 tint)`, and `setBorder(ImVec4 border)`.
  * **`ProgressBar`**:
      * Creation: `static ProgressBarPtr create()`.
      * Configuration: `ProgressBarPtr setValue(float v)` (clamped 0 to 1), `ProgressBarPtr setOverlayText(const std::string &text)`, `setSpanAvailWidth(bool)`, `setUseContainerWidth(bool)`.
  * **`Separator`**:
      * Enum Definitions: `SeparatorOrientation::Horizontal` and `Vertical`. `SeparatorType::Custom`, `Native`, and `Text`.
      * Creation: `static SeparatorPtr create()`.
      * Configuration: `setType(SeparatorType t)`, `setOrientation(SeparatorOrientation o)`, `setThickness(float t)`, `setAsRect(bool rect)`, `setColor(ImVec4 c)`, and `setText(const std::string &s)` (automatically promotes type to `Text`).
  * **`Card`**:
      * Creation: `static CardPtr create()`.
      * Configuration: `setChild(IControlPtr c)` dictates payload. `setPadding(float p)` injects `ImGuiStyleVar_WindowPadding` onto a constrained internal ImGui child context. `setFillHeight(bool fill)` forces expansion in the Y axis.

-----

# 7\. Dialogs System

MUI offers asynchronous wrappers overriding the application flow with sequential blocking or overlay operations.

## 7.1 Message Boxes

Message boxes queue into `App::activeMessageBoxes` using an `ActiveMessageBox` definition specifying `title`, `message`, `type` (`MessageBoxType::Info`, `Warning`, `Error`, `Question`), and a vector of `MessageBoxButton` structs mapped to callback functions.

  * **Utility Methods**:
      * `static void msgBox(const std::string &title, const std::string &message)` and `msgBoxInfo(...)` generate `Info` boxes defaulting to an "OK" confirmation button.
      * `static void msgBoxWarning(...)` triggers Amber/Yellow visual iconography.
      * `static void msgBoxError(...)` triggers Red visual iconography.
      * `static void msgBoxConfirm(const std::string &title, const std::string &message, std::function<void()> on_ok, std::function<void()> on_cancel)` maps two explicit functions.
      * `static void msgBoxQuestion(...)` mimics confirmation but alters nomenclature to "Yes/No".
      * `static void msgBoxCustom(const std::string &title, const std::string &message, MessageBoxType type, std::vector<MessageBoxButton> &&buttons)`.

## 7.2 File Dialog System

MUI integrates a complex asynchronous file traversal GUI relying on `std::filesystem` and multithreaded image preview logic. Callbacks are intercepted by the main event loop via `Dialogs::processDialogs()`.

  * **Standard File Access**:
      * `static void openFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, std::function<void()> on_cancel, const std::string &startingDir = "")` parses target directories against wildcard filtering (e.g., `"{.*}"`).
      * `static void openFiles(const std::string &title, const std::string &filter, std::function<void(const std::vector<std::string> &)> on_ok, ...)` executes in multiselect configuration mode.
      * `static void saveFile(const std::string &title, const std::string &filter, std::function<void(const std::string &)> on_ok, ...)` modifies target behavior to inject extensions and spawn file confirmation loops.

# 8. Deep Dive: Custom Widget Rendering and Internal Math

MUI does not just wrap ImGui; it implements custom drawing logic for widgets that do not exist natively in ImGui.

## 8.1 ToggleSwitch Internals
The `ToggleSwitch` bypasses native ImGui widgets to draw a custom iOS-style switch using the ImGui `DrawList`.
* **Dimensional Math:** The base height is calculated as `ImGui::GetFrameHeight() * scale`. The width is strictly locked to `switch_height * 1.8f`.
* **Hitbox Registration:** It calculates a total bounding box (`total_bb`) encompassing the switch and the label (if present), registers it via `ImGui::ItemAdd()`, and processes clicks using `ImGui::ButtonBehavior()`.
* **Rendering:** * The background track is drawn using `window->DrawList->AddRectFilled()` with a corner radius of `switch_height * 0.50f`.
    * The circular thumb is drawn using `window->DrawList->AddCircleFilled()`. Its X-position interpolates between the left and right sides of the track based on a ternary float `t = checked ? 1.0f : 0.0f`.

## 8.2 RangeSlider Internals
The `RangeSlider` is a highly complex custom widget requiring two independent grab handles operating on a single track.
* **State Tracking:** Because ImGui's native sliders only support one value, MUI uses `ImGui::GetStateStorage()->GetInt/SetInt` tied to the slider's unique ID to track which handle is currently active (`1` for minimum, `2` for maximum, `0` for none).
* **Interaction Logic:** * When clicked (`g.ActiveIdIsJustActivated`), it calculates the absolute distance from the mouse X position to both handles (`grabMinX` and `grabMaxX`) to determine which handle to activate.
    * It enforces boundary constraints: `currentMin` cannot exceed `currentMax`, and `currentMax` cannot drop below `currentMin`.
* **Rendering:** * The track is drawn via `ImGui::RenderFrame()`.
    * A colored highlight box is drawn strictly between `grabMinX` and `grabMaxX` using `ImGuiCol_Header`.
    * Text is rendered perfectly centered over the track using `ImGui::RenderTextClipped` with a format of `min - max`.

## 8.3 Entry Context Menu and Clipboard Sanitization
The `Entry` widget provides a deeply integrated context menu triggered via `ImGui::BeginPopupContextItem("##context_menu")`.
* **Cut/Copy:** Retrieves the selection bounds (`selStart`, `selEnd`), extracts the substring, and pushes it to `ImGui::SetClipboardText()`.
* **Paste Sanitization:** When pasting into a single-line `Entry`, the clipboard string is heavily sanitized before insertion:
    * If `ImGuiInputTextFlags_CharsNoBlank` is set, it actively strips `\n`, `\r`, space, and `\t` via `std::remove_if`.
    * If `ImGuiInputTextFlags_Password` is set, it strips `\n` and `\r`.
    * Otherwise, it replaces `\n` and `\r` with spaces via `std::replace_if`.

## 8.4 IconStack Multi-Pass Rendering
To ensure perfectly uniform button sizes when rendering a horizontal list of different icons, `IconStack` performs a two-pass calculation.
* **Pass 1:** Iterates through all internal `icons` and uses `ImGui::CalcTextSize()` to find the maximum `max_icon_width` across the entire set.
* **Pass 2:** Renders each button strictly using `button_width = max_icon_width + style.FramePadding.x * 2.0f`, manually calculating the `icon_pos` to perfectly center the specific icon glyph within this forced uniform boundary.

---

# 9. Exhaustive Theme Engine and Font Loading

The `mui::Theme` class handles cross-platform font resolution, TOML deserialization, and advanced color matrices.

## 9.1 Cross-Platform System Font Resolution
`Theme::loadSystemFont()` dynamically hunts for the host OS's primary sans-serif font to ensure native integration.
* **Windows:** Uses the Win32 API `SHGetFolderPathA(NULL, CSIDL_FONTS, ...)` to locate `segoeui.ttf`. It falls back to `C:\Windows\Fonts\segoeui.ttf` if the API fails.
* **macOS:** Hardcodes the path to `/System/Library/Fonts/Supplemental/Helvetica.ttc` due to Apple's stable filesystem architecture.
* **Linux:** Attempts to use the `fontconfig` library (`FcInitLoadConfigAndFonts`, `FcFontMatch`) to request the system's default `sans-serif` font dynamically. If `fontconfig` is unavailable, it cascades through a hardcoded array of known paths (e.g., Ubuntu, DejaVu, Liberation, Noto) testing for existence via `std::ifstream`.
* **Icon Merging:** After the base font is loaded, FontAwesome 6 (`fa_solid_900`) is loaded from compressed memory. The `ImFontConfig.MergeMode` is set to `true`, and `GlyphMinAdvanceX` is locked to the `fontSize` to force the icons to render as monospaced characters directly inside the primary text font.

## 9.2 TOML Deserialization Engine
When `App::setTheme(filepath, themeName)` is called, MUI parses a TOML file.
* **Case-Insensitive Parsing:** MUI implements a custom lambda `findCaseInsensitive` that iterates over TOML tables and performs a `std::tolower` character-by-character comparison, ensuring that keys like `WindowBg` or `windowbg` map correctly.
* **Style Variables:** Supports parsing floats, `ImVec2` (as JSON-like arrays `[x, y]`), and enumerations.
    * `ImGuiDir` is parsed from strings `"none"`, `"left"`, `"right"`, `"up"`, `"down"`.
    * `ImGuiTreeNodeFlags` and `ImGuiHoveredFlags` are parsed from arrays of strings, iterating and applying bitwise OR `|` operations against an internal `std::unordered_map` of flag names.
* **Color Parsing:** * Accepts arrays of 3 or 4 floats (RGBA).
    * Accepts Hex strings (`#RRGGBB` or `#RRGGBBAA`), leveraging `sscanf` to parse the hex pairs and dividing by `255.0f` to map them into ImGui's `0.0f - 1.0f` color space.
    * Supports legacy/aliased keys like `tabactive` (mapping to `ImGuiCol_TabSelected`) and `navhighlight` (mapping to `ImGuiCol_NavCursor`).

---

# 10. File Dialog Internals and Multithreading

The `mui_dlg::FileDialog` system is the most complex component of MUI, heavily utilizing multithreading and OS-level APIs.

## 10.1 Multithreaded Image Preview Loader
To prevent the UI from freezing when navigating directories with large images, previews are loaded asynchronously.
* **Architecture:** Uses a single background `std::thread` (`m_previewLoader`), synchronized via `std::mutex` and a `std::condition_variable` (`m_previewCv`).
* **Queuing:** When `m_refreshIconPreview()` triggers at a zoom level `>= 5.0f`, it scans the directory for `.png`, `.jpg`, `.jpeg`, `.bmp`, or `.tga` files, pushing their `std::filesystem::path` into `m_previewQueue`.
* **Worker Execution:** The thread waits on the condition variable until the queue is populated or the exit flag is set. It pops paths (LIFO order for responsiveness), uses `stb_image` (`stbi_load`) to decode the image into a raw buffer, and pushes the buffer into `m_previewResults`.
* **Main Thread Resolution:** `m_processPreviewResults()` runs sequentially on the main thread during rendering. It pops results, invokes the `CreateTexture` callback to upload the buffer to the GPU (OpenGL or SDL), frees the raw RAM buffer (`stbi_image_free`), and attaches the texture ID to the corresponding `FileData` struct.

## 10.2 Cross-Platform Icon and Drive Resolution
* **Windows (`_WIN32`):**
    * `m_getIcon` uses `SHGetFileInfoW` with `SHGFI_ICON | SHGFI_LARGEICON` to fetch the native Windows shell icon. It extracts the `HBITMAP`, converts it to a raw `uint8_t` array via `GetBitmapBits`, and pushes it to the GPU via `CreateTexture`. It caches icons by `iIcon` index to prevent VRAM duplication.
    * `m_initSystemFavorites` queries the OS for the active user via `GetUserNameW`, constructs standard paths (Desktop, Documents, etc.), and uses `GetLogicalDrives()` bitmasking to map mounted physical drives (A: through Z:).
* **POSIX (Linux/macOS):**
    * `m_getIcon` always returns `nullptr`, forcing the UI to fallback to FontAwesome glyphs (`ICON_FA_FOLDER`, `ICON_FA_FILE`) natively.
    * `m_initSystemFavorites` uses `geteuid` and `getpwuid` to resolve the current user's `/home/username` directory, probing for standard directories via `std::filesystem::exists`.

## 10.3 Sorting and Data Modeling
* The directory contents are stored in a `std::vector<FileData>`.
* `FileData` captures `IsDirectory`, `Size` (`st_size`), and `DateModified` (`st_mtime` from `sys/stat.h` for speed, falling back to `std::filesystem` if stat fails).
* `m_sortContent` utilizes `std::partition` to strictly separate Directories from Files first, ensuring folders always appear at the top. It then applies `std::sort` independently to both partitions based on the active `ImGuiTableSortSpecs` (Name, Date, or Size).

---

# 11. Exhaustive Internal Structs and Flags

To fully construct and manipulate MUI components, developers must understand the underlying internal structs and encapsulated ImGui flags.

## 11.1 Internal Layout Structs
* **`BoxChild`** (in `Box`): Contains `IControlPtr control` and `bool stretchy`.
* **`flow_internal`** (in `FlowBox`): Tracks `lastpos`, `lastline`, `lastKnownHeight`, and `lastKnownWidth` to calculate wrap boundaries dynamically.
* **`TabPage`** (in `Tab`): Contains `std::string name`, `IControlPtr control`, and `bool margined`.
* **`Cell`** (in `Grid`): Contains `IControlPtr control` and `int colSpan`.
* **`TableColumn`** (in `Table`): Contains `std::string name`, `float weight`, and `bool fixedWidth`.
* **`IconBtn`** (in `IconStack`): Contains `std::string icon`, `std::string tooltip`, and `bool text_hover`.

## 11.2 Exhaustive Encapsulated Flags
MUI widgets expose setter methods that directly manipulate binary ImGui flags.

* **`Entry` Flags (`ImGuiInputTextFlags`)**:
    * `setReadOnly` -> `ReadOnly`
    * `setAutoSelectAll` -> `AutoSelectAll`
    * `setNoSpaces` -> `CharsNoBlank`
    * `setCharsDecimal` -> `CharsDecimal`
    * `setCharsHexadecimal` -> `CharsHexadecimal`
    * `setCharsUppercase` -> `CharsUppercase`
    * `setAllowTabInput` -> `AllowTabInput`
    * `setCtrlEnterForNewLine` -> `CtrlEnterForNewLine`
    * `PasswordEntry` strictly enforces `Password`.

* **`ComboBox` Flags (`ImGuiComboFlags`)**:
    * `setPopupAlignLeft` -> `PopupAlignLeft`
    * `setHeightSmall` -> `HeightSmall`
    * `setHeightRegular` -> `HeightRegular`
    * `setHeightLarge` -> `HeightLarge`
    * `setHeightLargest` -> `HeightLargest`
    * `setNoArrowButton` -> `NoArrowButton`
    * `setNoPreview` -> `NoPreview`
    * `setCustomPreview` -> `WidthFitPreview`

* **`Table` Flags (`ImGuiTableFlags`)**:
    * Always applied default: `Borders | RowBg | ScrollY`.
    * `spanAvailWidth` -> injects `SizingStretchProp`.
    * `setResizable` -> `Resizable`
    * `setReorderable` -> `Reorderable`
    * `setHideable` -> `Hideable`
    * `setNoSavedSettings` -> `NoSavedSettings`
    * `setContextMenuInBody` -> `ContextMenuInBody`
    * `setScrollX` -> `ScrollX`
    * `setSortable` -> `Sortable`

* **`Tab` Flags (`ImGuiTabBarFlags`)**:
    * Always applied default: `TabListPopupButton`.
    * `setReorderable` -> `Reorderable`
    * `setAutoSelectNewTabs` -> `AutoSelectNewTabs`
    * `setNoCloseWithMiddleMouseButton` -> `NoCloseWithMiddleMouseButton`
    * `setNoTabListScrollingButtons` -> `NoTabListScrollingButtons`
    * `setNoTooltip` -> `NoTooltip`
    * `setFittingPolicyScroll` -> `FittingPolicyScroll`
    * `setFittingPolicyResizeDown` -> `FittingPolicyResizeDown`