#pragma once
#include <ui.h>
#include <string>
#include <functional>
#include <stdexcept>
#include <vector>
#include <memory>
#include <thread>

namespace mui
{

    // --- Core Application Management ---
    class App
    {
    private:
        static std::thread::id mainThreadId;

    public:
        static void init();
        static void run();
        static void quit();
        static void queueMain(std::function<void()> callback);
        
        // Ensures the current thread is the thread that called init()
        static void assertMainThread();
    };

    // --- Base Control Class ---
    class Control
    {
    protected:
        uiControl *handle = nullptr;
        bool ownsHandle = true; // Flag for RAII ownership enforcement

        // Traverses the C++ tree to neutralize pointers when the C UI tree is destroyed
        virtual void onHandleDestroyed();

    public:
        Control() = default;
        virtual ~Control();

        // Hard enforce RAII: non-copyable and non-movable
        Control(const Control&) = delete;
        Control& operator=(const Control&) = delete;
        Control(Control&&) = delete;
        Control& operator=(Control&&) = delete;

        uiControl *getHandle() const;
        void show();
        void hide();
        void setEnabled(bool enabled);
        bool isEnabled() const;
        
        void releaseOwnership();
        void acquireOwnership();
        void verifyState() const;
        
        // Allow containers to trigger onHandleDestroyed on their children
        friend class Window;
        friend class Box;
        friend class Group;
        friend class Tab;
    };

    // --- Dialogs (Static Helpers) ---
    class Dialogs
    {
    public:
        static void msgBox(Control &parent, const std::string &title, const std::string &description);
        static void msgBoxError(Control &parent, const std::string &title, const std::string &description);
        static std::string openFile(Control &parent);
    };

    // --- Window ---
    class Window : public Control
    {
    private:
        uiWindow *win = nullptr;
        std::shared_ptr<Control> child; // Keeps the child C++ object alive
        std::function<bool()> onClosingCb;

        static int onClosingStub(uiWindow *w, void *data);

    protected:
        void onHandleDestroyed() override;

    public:
        Window(const std::string &title, int width, int height, bool hasMenubar = false);
        void setChild(std::shared_ptr<Control> child);
        void setMargined(bool margined);
        void onClosing(std::function<bool()> cb);
    };

    // --- Containers ---
    class Box : public Control
    {
    protected:
        uiBox *box;
        std::vector<std::shared_ptr<Control>> children; // Keeps children alive
        Box(uiBox *b);
        
        void onHandleDestroyed() override;

    public:
        void append(std::shared_ptr<Control> child, bool stretchy = false);
        void deleteChild(int index);
        void setPadded(bool padded);
    };

    class VBox : public Box
    {
    public:
        VBox();
    };
    class HBox : public Box
    {
    public:
        HBox();
    };

    class Group : public Control
    {
    private:
        uiGroup *group;
        std::shared_ptr<Control> child;
        
    protected:
        void onHandleDestroyed() override;

    public:
        Group(const std::string &title);
        void setChild(std::shared_ptr<Control> child);
        void setMargined(bool margined);
    };

    class Tab : public Control
    {
    private:
        uiTab *tab;
        std::vector<std::shared_ptr<Control>> children;

    protected:
        void onHandleDestroyed() override;

    public:
        Tab();
        void append(const std::string &name, std::shared_ptr<Control> child);
        void setMargined(int page, bool margined);
    };

    // --- Data Entry & Interaction Widgets ---
    class Label : public Control
    {
        uiLabel *label;
    public:
        Label(const std::string &text);
        void setText(const std::string &text);
    };

    class Button : public Control
    {
        uiButton *btn;
        std::function<void()> onClickCb;
        static void onClickedStub(uiButton *b, void *data);

    public:
        Button(const std::string &text);
        void onClick(std::function<void()> cb);
    };

    class Entry : public Control
    {
        uiEntry *entry;
        std::function<void()> onChangedCb;
        static void onChangedStub(uiEntry *e, void *data);

    protected:
        Entry(uiEntry *e);

    public:
        Entry();
        std::string getText() const;
        void setText(const std::string &text);
        void setReadOnly(bool readOnly);
        void onChanged(std::function<void()> cb);
    };

    class PasswordEntry : public Entry
    {
    public:
        PasswordEntry();
    };

    class Checkbox : public Control
    {
        uiCheckbox *checkbox;
        std::function<void()> onToggledCb;
        static void onToggledStub(uiCheckbox *c, void *data);

    public:
        Checkbox(const std::string &text);
        bool isChecked() const;
        void setChecked(bool checked);
        void onToggled(std::function<void()> cb);
    };

    class Slider : public Control
    {
        uiSlider *slider;
        std::function<void()> onChangedCb;
        static void onChangedStub(uiSlider *s, void *data);

    public:
        Slider(int min, int max);
        int getValue() const;
        void setValue(int val);
        void onChanged(std::function<void()> cb);
    };

    class ProgressBar : public Control
    {
        uiProgressBar *progress;
    public:
        ProgressBar();
        void setValue(int val);
    };

} // namespace mui