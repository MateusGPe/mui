#pragma once
#include "include/mui.hpp"
#include <string>
#include <memory>
#include <vector>
#include <initializer_list>
#include <functional>
#include <tuple>
#include <type_traits>

namespace mui
{
    // ==========================================
    // 0. OBSERVABLE ALIASES & FACTORIES
    // ==========================================
    // These aliases make passing observables to functions clean and highly readable.
    using ObsBool = std::shared_ptr<Observable<bool>>;
    using ObsInt = std::shared_ptr<Observable<int>>;
    using ObsFloat = std::shared_ptr<Observable<float>>;
    using ObsString = std::shared_ptr<Observable<std::string>>;
    using ObsVec2 = std::shared_ptr<Observable<std::pair<float, float>>>;
    using ObsColor = std::shared_ptr<Observable<std::array<float, 4>>>;

    // Factory namespace for rapid, inline creation of observables
    namespace MakeObs
    {
        inline ObsBool Bool(bool v = false) { return std::make_shared<Observable<bool>>(v); }
        inline ObsInt Int(int v = 0) { return std::make_shared<Observable<int>>(v); }
        inline ObsFloat Float(float v = 0.0f) { return std::make_shared<Observable<float>>(v); }
        inline ObsString String(const std::string &v = "") { return std::make_shared<Observable<std::string>>(v); }
        inline ObsVec2 Range(float min = 0.0f, float max = 1.0f) { return std::make_shared<Observable<std::pair<float, float>>>(std::make_pair(min, max)); }
        inline ObsColor Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) { return std::make_shared<Observable<std::array<float, 4>>>(std::array<float, 4>{r, g, b, a}); }
    }

    // ==========================================
    // 1. STRUCTURAL TAGS (For Operator Routing)
    // ==========================================

    struct Stretch
    {
        IControlPtr control;
        template <typename T, typename = std::enable_if_t<std::is_convertible_v<std::shared_ptr<T>, IControlPtr>>>
        explicit Stretch(std::shared_ptr<T> c) : control(std::move(c))
        {
        }
    };

    template <typename T>
    struct Ref
    {
        std::shared_ptr<T> &ptr;
    };

    struct GridCell
    {
        IControlPtr control;
        int row, col, colSpan;
        template <typename T>
        GridCell(std::shared_ptr<T> c, int r, int c_idx, int cs = 1) : control(std::move(c)), row(r), col(c_idx), colSpan(cs)
        {
        }
    };

    struct TabPage
    {
        std::string name;
        IControlPtr control;
        bool margined;
        template <typename T>
        TabPage(std::string n, std::shared_ptr<T> c, bool m = false) : name(std::move(n)), control(std::move(c)), margined(m)
        {
        }
    };

    struct TableCol
    {
        std::string name;
        float weight;
        bool fixed;
        TableCol(std::string n, float w = 1.0f, bool f = false) : name(std::move(n)), weight(w), fixed(f)
        {
        }
    };

    struct TableRow
    {
        std::vector<IControlPtr> items;
        TableRow(std::initializer_list<IControlPtr> i) : items(i)
        {
        }
    };

    struct PropCat
    {
        std::string name;
        bool defaultOpen;
        PropCat(std::string n, bool o = true) : name(std::move(n)), defaultOpen(o)
        {
        }
    };

    struct PropItem
    {
        std::string label;
        IControlPtr editor;
        template <typename T>
        PropItem(std::string l, std::shared_ptr<T> e) : label(std::move(l)), editor(std::move(e))
        {
        }
    };

    struct SplitterLeft
    {
        IControlPtr control;
        template <typename T>
        explicit SplitterLeft(std::shared_ptr<T> c) : control(std::move(c))
        {
        }
    };

    struct SplitterRight
    {
        IControlPtr control;
        template <typename T>
        explicit SplitterRight(std::shared_ptr<T> c) : control(std::move(c))
        {
        }
    };

    struct IconDef
    {
        std::string icon;
        std::function<void()> cb;
        std::string tip;
        bool textHover;
        IconDef(std::string i, std::function<void()> c = nullptr, std::string t = "", bool h = false)
            : icon(std::move(i)), cb(std::move(c)), tip(std::move(t)), textHover(h)
        {
        }
    };

    template <typename Sig, typename Cb>
    struct ObserveDef
    {
        Sig &signal;
        Cb callback;
    };

    // ==========================================
    // 2. STREAM OPERATORS (The Fluent Pipeline)
    // ==========================================

    // Containers (VBox, HBox, FlowBox)
    template <typename BoxType, typename std::enable_if_t<std::is_base_of_v<IControl, typename BoxType::element_type>, int> = 0>
    inline BoxType operator<<(BoxType box, IControlPtr child)
    {
        box->append(child, false);
        return box;
    }

    template <typename BoxType, typename std::enable_if_t<std::is_base_of_v<IControl, typename BoxType::element_type>, int> = 0>
    inline BoxType operator<<(BoxType box, const Stretch &child)
    {
        box->append(child.control, true);
        return box;
    }

    // Single-Child Hosts (Group, Card, Window)
    inline GroupPtr operator<<(GroupPtr group, IControlPtr child)
    {
        group->setChild(child);
        return group;
    }
    inline CardPtr operator<<(CardPtr card, IControlPtr child)
    {
        card->setChild(child);
        return card;
    }
    inline WindowPtr operator<<(WindowPtr win, IControlPtr child)
    {
        win->setChild(child);
        return win;
    }

    // Tree Node
    inline TreeNodePtr operator<<(TreeNodePtr node, IControlPtr child)
    {
        node->append(child);
        return node;
    }

    // Splitter
    inline SplitterViewPtr operator<<(SplitterViewPtr s, const SplitterLeft &l)
    {
        s->setPanel1(l.control);
        return s;
    }
    inline SplitterViewPtr operator<<(SplitterViewPtr s, const SplitterRight &r)
    {
        s->setPanel2(r.control);
        return s;
    }

    // Grid
    inline GridPtr operator<<(GridPtr grid, const GridCell &cell)
    {
        grid->append(cell.control, cell.row, cell.col, cell.colSpan);
        return grid;
    }

    // Tab
    inline TabPtr operator<<(TabPtr tab, const TabPage &page)
    {
        tab->append(page.name, page.control);
        tab->setMargined(tab->getNumPages() - 1, page.margined);
        return tab;
    }

    // Table
    inline TablePtr operator<<(TablePtr table, const TableCol &col)
    {
        table->addColumn(col.name, col.weight, col.fixed);
        return table;
    }
    inline TablePtr operator<<(TablePtr table, const TableRow &row)
    {
        table->addRow(row.items);
        return table;
    }

    // Property Grid
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PropCat &cat)
    {
        pg->addCategory(cat.name, cat.defaultOpen);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PropItem &item)
    {
        pg->addProperty(item.label, item.editor);
        return pg;
    }

    // Lists & Stacks
    inline ComboBoxPtr operator<<(ComboBoxPtr box, const std::string &item)
    {
        box->append(item);
        return box;
    }
    inline ListBoxPtr operator<<(ListBoxPtr box, const std::string &item)
    {
        box->append(item);
        return box;
    }
    inline IconStackPtr operator<<(IconStackPtr stack, const IconDef &def)
    {
        stack->add(def.icon, def.cb, def.tip, def.textHover);
        return stack;
    }

    // This calls widget->observe() behind the scenes, ensuring the [[nodiscard]]
    // connection is stored in the widget's protected m_connections array.
    template <typename T, typename Sig, typename Cb, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const ObserveDef<Sig, Cb> &def)
    {
        return control->observe(def.signal, def.callback);
    }

    template <typename T, typename U, typename = std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int>>
    inline T operator<<(T control, const Ref<U> &ref)
    {
        // Use dynamic_pointer_cast for safe downcasting
        ref.ptr = std::dynamic_pointer_cast<U>(control);
        return control;
    }

    // ==========================================
    // 3. FACTORY NAMESPACE
    // ==========================================
    namespace UI
    {
        // --- Windows & Base Layouts ---
        inline WindowPtr Window(const std::string &title, int w, int h, bool hasMenubar = false)
        {
            return mui::Window::create(title, w, h, hasMenubar);
        }
        inline VBoxPtr VBox(bool padded = true, bool scrollable = false)
        {
            return mui::VBox::create()->setPadded(padded)->setScrollable(scrollable);
        }
        inline HBoxPtr HBox(bool padded = true, bool scrollable = false)
        {
            return mui::HBox::create()->setPadded(padded)->setScrollable(scrollable);
        }
        inline FlowBoxPtr FlowBox(mui::FlowBox::Align align = mui::FlowBox::Align::Left)
        {
            return mui::FlowBox::create()->setAlign(align);
        }
        inline CardPtr Card(float padding = 8.0f, bool fillHeight = false)
        {
            return mui::Card::create()->setPadding(padding)->setFillHeight(fillHeight);
        }
        inline GroupPtr Group(const std::string &title, bool defaultOpen = true, bool margined = true)
        {
            return mui::Group::create(title)->defaultOpen(defaultOpen)->setMargined(margined);
        }
        inline SplitterViewPtr Splitter(SplitterOrientation o = SplitterOrientation::Horizontal, float ratio = 0.5f)
        {
            return mui::SplitterView::create(o)->setSplitRatio(ratio);
        }
        inline TabPtr Tab()
        {
            return mui::Tab::create();
        }
        inline PropertyGridPtr PropertyGrid(float nameColWidth = 150.0f)
        {
            return mui::PropertyGrid::create()->setNameColumnWidth(nameColWidth);
        }
        inline TreeNodePtr TreeNode(const std::string &label, bool defaultOpen = false, const std::string &iconText = "")
        {
            auto n = mui::TreeNode::create(label)->setDefaultOpen(defaultOpen);
            if (!iconText.empty())
                n->setIconText(iconText);
            return n;
        }

        // --- Grid & Table Initializers ---
        inline GridPtr Grid(std::initializer_list<float> colWeights = {})
        {
            auto g = mui::Grid::create();
            int c = 0;
            for (float w : colWeights)
                g->setColumnWeight(c++, w);
            return g;
        }
        inline TablePtr Table(std::initializer_list<TableCol> cols = {})
        {
            auto t = mui::Table::create();
            for (const auto &c : cols)
                t->addColumn(c.name, c.weight, c.fixed);
            return t;
        }

        // --- Controls ---
        inline LabelPtr Label(const std::string &text, LabelFormat format = LabelFormat::Normal)
        {
            return mui::Label::create(text)->setFormat(format);
        }
        inline SeparatorPtr Separator(SeparatorType t = SeparatorType::Native, SeparatorOrientation o = SeparatorOrientation::Horizontal)
        {
            return mui::Separator::create()->setType(t)->setOrientation(o);
        }
        inline ProgressBarPtr ProgressBar(float value, const std::string &overlay = "")
        {
            return mui::ProgressBar::create()->setValue(value)->setOverlayText(overlay);
        }
        inline ImagePtr Image(ImTextureID tex, float w = 0, float h = 0)
        {
            return mui::Image::create(tex, w, h);
        }
        inline IconStackPtr IconStack()
        {
            return mui::IconStack::create();
        }

        inline ButtonPtr Button(const std::string &text, std::function<void()> onClick = nullptr)
        {
            auto b = mui::Button::create(text);
            if (onClick)
                b->onClick(std::move(onClick));
            return b;
        }
        inline IconButtonPtr IconButton(const std::string &iconText, const std::string &text = "", std::function<void()> onClick = nullptr)
        {
            auto b = mui::IconButton::create(text)->setIconText(iconText);
            if (onClick)
                b->onClick(std::move(onClick));
            return b;
        }

        inline CheckboxPtr Checkbox(const std::string &text, bool checked = false, std::function<void(bool)> onToggled = nullptr)
        {
            auto c = mui::Checkbox::create(text)->setChecked(checked);
            if (onToggled)
                c->onToggled(std::move(onToggled));
            return c;
        }
        inline RadioButtonPtr RadioButton(const std::string &text, bool checked = false, std::function<void(bool)> onToggled = nullptr)
        {
            auto r = mui::RadioButton::create(text)->setChecked(checked);
            if (onToggled)
                r->onToggled(std::move(onToggled));
            return r;
        }
        inline ToggleSwitchPtr ToggleSwitch(const std::string &text, bool checked = false, std::function<void(bool)> onToggled = nullptr)
        {
            auto t = mui::ToggleSwitch::create(text)->setChecked(checked);
            if (onToggled)
                t->onToggled(std::move(onToggled));
            return t;
        }

        inline EntryPtr Entry(const std::string &hint = "", std::function<void(const std::string &)> onEnter = nullptr)
        {
            auto e = mui::Entry::create()->setHint(hint);
            if (onEnter)
                e->onEnter(std::move(onEnter));
            return e;
        }
        inline PasswordEntryPtr PasswordEntry(const std::string &hint = "", std::function<void(const std::string &)> onEnter = nullptr)
        {
            auto e = mui::PasswordEntry::create();
            e->setHint(hint);
            if (onEnter)
                e->onEnter(std::move(onEnter));
            return e;
        }
        inline SearchEntryPtr SearchEntry(const std::string &hint = "Search...", std::function<void(const std::string &)> onEnter = nullptr)
        {
            auto e = mui::SearchEntry::create();
            e->setHint(hint);
            if (onEnter)
                e->onEnter(std::move(onEnter));
            return e;
        }

        inline SpinboxPtr Spinbox(int min, int max, int val, std::function<void()> onChanged = nullptr)
        {
            auto s = mui::Spinbox::create(min, max)->setValue(val);
            if (onChanged)
                s->onChanged(std::move(onChanged));
            return s;
        }
        inline SliderIntPtr SliderInt(int min, int max, int val, std::function<void()> onChanged = nullptr)
        {
            auto s = mui::SliderInt::create(min, max)->setValue(val);
            if (onChanged)
                s->onChanged(std::move(onChanged));
            return s;
        }
        inline SliderFloatPtr SliderFloat(float min, float max, float val, std::function<void()> onChanged = nullptr)
        {
            auto s = mui::SliderFloat::create(min, max)->setValue(val);
            if (onChanged)
                s->onChanged(std::move(onChanged));
            return s;
        }
        inline RangeSliderPtr RangeSlider(float min, float max, float vMin, float vMax, std::function<void(float, float)> onChanged = nullptr)
        {
            auto s = mui::RangeSlider::create(min, max)->setRange(vMin, vMax);
            if (onChanged)
                s->onChanged(std::move(onChanged));
            return s;
        }

        inline ComboBoxPtr ComboBox(int selected = 0, std::function<void(int)> onChanged = nullptr)
        {
            auto c = mui::ComboBox::create()->setSelectedIndex(selected);
            if (onChanged)
                c->onChanged(std::move(onChanged));
            return c;
        }
        inline ListBoxPtr ListBox(int selected = -1, std::function<void()> onSelected = nullptr)
        {
            auto l = mui::ListBox::create()->setSelected(selected);
            if (onSelected)
                l->onSelected(std::move(onSelected));
            return l;
        }
        inline BreadcrumbBarPtr BreadcrumbBar(const std::string &path, std::function<void(const std::string &)> onNavigated = nullptr)
        {
            auto b = mui::BreadcrumbBar::create(path);
            if (onNavigated)
                b->onPathNavigated(std::move(onNavigated));
            return b;
        }
        inline ColorEditPtr ColorEdit(float r, float g, float b, float a = 1.0f, std::function<void(const std::array<float, 4> &)> onChanged = nullptr)
        {
            auto c = mui::ColorEdit::create(r, g, b, a);
            if (onChanged)
                c->onChanged(std::move(onChanged));
            return c;
        }

        template <typename Sig, typename Cb>
        inline ObserveDef<Sig, Cb> Observe(Sig &signal, Cb cb)
        {
            return {signal, std::move(cb)};
        }

        template <typename T>
        inline Ref<T> Capture(std::shared_ptr<T> &ptr)
        {
            return {ptr};
        }

        inline LabelPtr LabelBind(ObsString obs, LabelFormat format = LabelFormat::Normal)
        {
            auto lbl = mui::Label::create(obs ? obs->get() : "")->setFormat(format);
            if (obs)
            {
                lbl->observe(
                    obs->onValueChanged,
                    [lbl](const std::string &val)
                    {
                        mui::App::queueMain(
                            [lbl, val]()
                            {
                                lbl->setText(val);
                            });
                    });
            }
            return lbl;
        }

        inline ProgressBarPtr ProgressBarBind(ObsFloat obs, const std::string &overlay = "")
        {
            auto pb = mui::ProgressBar::create()->setOverlayText(overlay);
            if (obs)
            {
                pb->setValue(obs->get());
                pb->observe(
                    obs->onValueChanged,
                    [pb](const float &val)
                    {
                        mui::App::queueMain(
                            [pb, val]()
                            {
                                pb->setValue(val);
                            });
                    });
            }
            return pb;
        }

        // --- Data Binding Binders ---
        inline CheckboxPtr CheckboxBind(const std::string &text, ObsBool obs)
        {
            return mui::Checkbox::create(text)->bind(obs);
        }
        inline ToggleSwitchPtr ToggleSwitchBind(const std::string &text, ObsBool obs)
        {
            return mui::ToggleSwitch::create(text)->bind(obs);
        }
        inline EntryPtr EntryBind(const std::string &hint, ObsString obs)
        {
            return mui::Entry::create()->setHint(hint)->bind(obs);
        }
        inline SpinboxPtr SpinboxBind(int min, int max, ObsInt obs)
        {
            return mui::Spinbox::create(min, max)->bind(obs);
        }
        inline SliderIntPtr SliderIntBind(int min, int max, ObsInt obs)
        {
            return mui::SliderInt::create(min, max)->bind(obs);
        }
        inline SliderFloatPtr SliderFloatBind(float min, float max, ObsFloat obs)
        {
            return mui::SliderFloat::create(min, max)->bind(obs);
        }
        inline RangeSliderPtr RangeSliderBind(float min, float max, ObsVec2 obs)
        {
            return mui::RangeSlider::create(min, max)->bind(obs);
        }
        inline ComboBoxPtr ComboBoxBind(ObsInt obs)
        {
            return mui::ComboBox::create()->bind(obs);
        }
        inline ListBoxPtr ListBoxBind(ObsInt obs)
        {
            return mui::ListBox::create()->bind(obs);
        }
        inline BreadcrumbBarPtr BreadcrumbBarBind(ObsString obs)
        {
            return mui::BreadcrumbBar::create()->bind(obs);
        }
        inline ColorEditPtr ColorEditBind(ObsColor obs)
        {
            return mui::ColorEdit::create()->bind(obs);
        }
        inline SplitterViewPtr SplitterBind(SplitterOrientation o, ObsFloat obs)
        {
            return mui::SplitterView::create(o)->bindSplitRatio(obs);
        }
        inline GroupPtr GroupBind(ObsBool openObs, ObsString titleObs = nullptr)
        {
            auto g = mui::Group::create("");
            if (openObs)
                g->bindOpen(openObs);
            if (titleObs)
                g->bindTitle(titleObs);
            return g;
        }
        inline TabPtr TabBind(ObsInt obs)
        {
            return mui::Tab::create()->bindSelected(obs);
        }
        inline TablePtr TableBind(ObsInt obs)
        {
            return mui::Table::create()->bind(obs);
        }
        inline TreeNodePtr TreeNodeBind(const std::string &label, ObsBool obs)
        {
            return mui::TreeNode::create(label)->bindSelected(obs);
        }

        // --- Compound Helpers ---
        inline HBoxPtr Parameter(const std::string &label, IControlPtr control, float labelWidth = 120.0f)
        {
            return HBox(true) << Label(label)->setWidth(labelWidth) << Stretch(control);
        }

        inline VBoxPtr RadioGroup(std::initializer_list<RadioButtonPtr> buttons)
        {
            mui::RadioButton::group(buttons);
            auto box = VBox();
            for (auto &b : buttons)
                box << b;
            return box;
        }
    }
} // namespace mui