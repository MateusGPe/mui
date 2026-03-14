#include "mui.hpp"

namespace mui
{

    // --- Core Application Management ---
    std::thread::id App::mainThreadId;

    void App::init()
    {
        mainThreadId = std::this_thread::get_id();
        uiInitOptions options = {};
        const char *err = uiInit(&options);
        if (err != nullptr)
        {
            // RAII cleanup of initialization error messages
            std::unique_ptr<const char, decltype(&uiFreeInitError)> errorStr(err, uiFreeInitError);
            throw std::runtime_error(std::string("Failed to initialize libui-ng: ") + errorStr.get());
        }
    }
    void App::run() { uiMain(); }
    void App::quit() { uiQuit(); }

    void App::assertMainThread()
    {
        if (std::this_thread::get_id() != mainThreadId)
        {
            throw std::runtime_error("MUI Error: UI interaction outside of the main thread.");
        }
    }

    void App::queueMain(std::function<void()> callback)
    {
        auto *cb = new std::function<void()>(std::move(callback));
        uiQueueMain([](void *data)
                    {
            // Take ownership back from the C-API to guarantee cleanup
            std::unique_ptr<std::function<void()>> func(static_cast<std::function<void()>*>(data));
            (*func)(); }, cb);
    }

    // --- Base Control Class ---
    Control::~Control()
    {
        if (handle && ownsHandle)
        {
            uiControlDestroy(handle);
        }
    }

    void Control::onHandleDestroyed()
    {
        handle = nullptr;
        ownsHandle = false;
    }

    void Control::verifyState() const
    {
        if (!handle)
        {
            throw std::runtime_error("MUI Error: Control handle has been destroyed.");
        }
        App::assertMainThread();
    }

    uiControl *Control::getHandle() const { return handle; }
    void Control::show()
    {
        verifyState();
        uiControlShow(handle);
    }
    void Control::hide()
    {
        verifyState();
        uiControlHide(handle);
    }
    void Control::setEnabled(bool enabled)
    {
        verifyState();
        enabled ? uiControlEnable(handle) : uiControlDisable(handle);
    }
    bool Control::isEnabled() const
    {
        verifyState();
        return uiControlEnabled(handle) != 0;
    }

    void Control::releaseOwnership() { ownsHandle = false; }
    void Control::acquireOwnership() { ownsHandle = true; }

    // --- Dialogs (Static Helpers) ---
    void Dialogs::msgBox(Control &parent, const std::string &title, const std::string &description)
    {
        parent.verifyState();
        uiMsgBox(uiWindow(parent.getHandle()), title.c_str(), description.c_str());
    }
    void Dialogs::msgBoxError(Control &parent, const std::string &title, const std::string &description)
    {
        parent.verifyState();
        uiMsgBoxError(uiWindow(parent.getHandle()), title.c_str(), description.c_str());
    }
    std::string Dialogs::openFile(Control &parent)
    {
        parent.verifyState();
        // RAII for char array return from openFile
        std::unique_ptr<char, decltype(&uiFreeText)> res(uiOpenFile(uiWindow(parent.getHandle())), uiFreeText);
        return res ? std::string(res.get()) : std::string();
    }

    // --- Window ---
    int Window::onClosingStub(uiWindow *w, void *data)
    {
        auto self = static_cast<Window *>(data);
        // If no callback is provided, default to closing the window.
        bool shouldClose = self->onClosingCb ? self->onClosingCb() : true;
        if (shouldClose)
        {
            // libui destroys the window and its children on returning 1.
            // We must cascade the destruction state through the C++ tree to prevent double-freeing.
            self->onHandleDestroyed();
        }
        return shouldClose ? 1 : 0;
    }

    Window::Window(const std::string &title, int width, int height, bool hasMenubar)
    {
        App::assertMainThread();
        win = uiNewWindow(title.c_str(), width, height, hasMenubar ? 1 : 0);
        handle = uiControl(win);
        uiWindowOnClosing(win, onClosingStub, this);
    }

    void Window::onHandleDestroyed()
    {
        Control::onHandleDestroyed();
        win = nullptr;
        if (child)
            child->onHandleDestroyed();
    }

    void Window::setChild(std::shared_ptr<Control> c)
    {
        verifyState();
        child = c;
        uiWindowSetChild(win, c->getHandle());
        c->releaseOwnership(); // libui takes memory ownership
    }
    void Window::setMargined(bool margined)
    {
        verifyState();
        uiWindowSetMargined(win, margined ? 1 : 0);
    }
    void Window::onClosing(std::function<bool()> cb)
    {
        verifyState();
        onClosingCb = std::move(cb);
    }

    // --- Containers ---
    Box::Box(uiBox *b) : box(b) { handle = uiControl(b); }

    void Box::onHandleDestroyed()
    {
        Control::onHandleDestroyed();
        box = nullptr;
        for (auto &c : children)
            c->onHandleDestroyed();
    }

    void Box::append(std::shared_ptr<Control> child, bool stretchy)
    {
        verifyState();
        children.push_back(child);
        uiBoxAppend(box, child->getHandle(), stretchy ? 1 : 0);
        child->releaseOwnership();
    }
    void Box::deleteChild(int index)
    {
        verifyState();
        if (index >= 0 && index < children.size())
        {
            children[index]->acquireOwnership(); // Re-claim ownership since it is removed from libui
            uiBoxDelete(box, index);
            children.erase(children.begin() + index);
        }
    }
    void Box::setPadded(bool padded)
    {
        verifyState();
        uiBoxSetPadded(box, padded ? 1 : 0);
    }

    VBox::VBox() : Box(uiNewVerticalBox()) { App::assertMainThread(); }
    HBox::HBox() : Box(uiNewHorizontalBox()) { App::assertMainThread(); }

    Group::Group(const std::string &title)
    {
        App::assertMainThread();
        group = uiNewGroup(title.c_str());
        handle = uiControl(group);
    }
    void Group::onHandleDestroyed()
    {
        Control::onHandleDestroyed();
        group = nullptr;
        if (child)
            child->onHandleDestroyed();
    }
    void Group::setChild(std::shared_ptr<Control> c)
    {
        verifyState();
        child = c;
        uiGroupSetChild(group, c->getHandle());
        c->releaseOwnership();
    }
    void Group::setMargined(bool margined)
    {
        verifyState();
        uiGroupSetMargined(group, margined ? 1 : 0);
    }

    Tab::Tab()
    {
        App::assertMainThread();
        tab = uiNewTab();
        handle = uiControl(tab);
    }
    void Tab::onHandleDestroyed()
    {
        Control::onHandleDestroyed();
        tab = nullptr;
        for (auto &c : children)
            c->onHandleDestroyed();
    }
    void Tab::append(const std::string &name, std::shared_ptr<Control> child)
    {
        verifyState();
        children.push_back(child);
        uiTabAppend(tab, name.c_str(), child->getHandle());
        child->releaseOwnership();
    }
    void Tab::setMargined(int page, bool margined)
    {
        verifyState();
        uiTabSetMargined(tab, page, margined ? 1 : 0);
    }

    // --- Data Entry & Interaction Widgets ---
    Label::Label(const std::string &text)
    {
        App::assertMainThread();
        label = uiNewLabel(text.c_str());
        handle = uiControl(label);
    }
    void Label::setText(const std::string &text)
    {
        verifyState();
        uiLabelSetText(label, text.c_str());
    }

    void Button::onClickedStub(uiButton *b, void *data)
    {
        auto self = static_cast<Button *>(data);
        if (self->onClickCb)
            self->onClickCb();
    }

    Button::Button(const std::string &text)
    {
        App::assertMainThread();
        btn = uiNewButton(text.c_str());
        handle = uiControl(btn);
        uiButtonOnClicked(btn, onClickedStub, this);
    }
    void Button::onClick(std::function<void()> cb)
    {
        verifyState();
        onClickCb = std::move(cb);
    }

    void Entry::onChangedStub(uiEntry *e, void *data)
    {
        auto self = static_cast<Entry *>(data);
        if (self->onChangedCb)
            self->onChangedCb();
    }

    Entry::Entry(uiEntry *e) : entry(e)
    {
        handle = uiControl(entry);
        uiEntryOnChanged(entry, onChangedStub, this);
    }

    Entry::Entry() : Entry(uiNewEntry()) { App::assertMainThread(); }
    std::string Entry::getText() const
    {
        verifyState();
        std::unique_ptr<char, decltype(&uiFreeText)> cstr(uiEntryText(entry), uiFreeText);
        return cstr ? std::string(cstr.get()) : std::string();
    }
    void Entry::setText(const std::string &text)
    {
        verifyState();
        uiEntrySetText(entry, text.c_str());
    }
    void Entry::setReadOnly(bool readOnly)
    {
        verifyState();
        uiEntrySetReadOnly(entry, readOnly ? 1 : 0);
    }
    void Entry::onChanged(std::function<void()> cb)
    {
        verifyState();
        onChangedCb = std::move(cb);
    }

    PasswordEntry::PasswordEntry() : Entry(uiNewPasswordEntry()) { App::assertMainThread(); }

    void Checkbox::onToggledStub(uiCheckbox *c, void *data)
    {
        auto self = static_cast<Checkbox *>(data);
        if (self->onToggledCb)
            self->onToggledCb();
    }

    Checkbox::Checkbox(const std::string &text)
    {
        App::assertMainThread();
        checkbox = uiNewCheckbox(text.c_str());
        handle = uiControl(checkbox);
        uiCheckboxOnToggled(checkbox, onToggledStub, this);
    }
    bool Checkbox::isChecked() const
    {
        verifyState();
        return uiCheckboxChecked(checkbox) != 0;
    }
    void Checkbox::setChecked(bool checked)
    {
        verifyState();
        uiCheckboxSetChecked(checkbox, checked ? 1 : 0);
    }
    void Checkbox::onToggled(std::function<void()> cb)
    {
        verifyState();
        onToggledCb = std::move(cb);
    }

    void Slider::onChangedStub(uiSlider *s, void *data)
    {
        auto self = static_cast<Slider *>(data);
        if (self->onChangedCb)
            self->onChangedCb();
    }

    Slider::Slider(int min, int max)
    {
        App::assertMainThread();
        slider = uiNewSlider(min, max);
        handle = uiControl(slider);
        uiSliderOnChanged(slider, onChangedStub, this);
    }
    int Slider::getValue() const
    {
        verifyState();
        return uiSliderValue(slider);
    }
    void Slider::setValue(int val)
    {
        verifyState();
        uiSliderSetValue(slider, val);
    }
    void Slider::onChanged(std::function<void()> cb)
    {
        verifyState();
        onChangedCb = std::move(cb);
    }

    ProgressBar::ProgressBar()
    {
        App::assertMainThread();
        progress = uiNewProgressBar();
        handle = uiControl(progress);
    }
    void ProgressBar::setValue(int val)
    {
        verifyState();
        uiProgressBarSetValue(progress, val);
    }

} // namespace mui