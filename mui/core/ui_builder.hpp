#pragma once
#include "include/mui.hpp"
#include <string>
#include <memory>
#include <array>
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
        float weight;
        template <typename T, typename = std::enable_if_t<std::is_convertible_v<std::shared_ptr<T>, IControlPtr>>>
        explicit Stretch(std::shared_ptr<T> c, float w = 1.0f) : control(std::move(c)), weight(w)
        {
        }
    };

    struct Auto
    {
        IControlPtr control;
        template <typename T, typename = std::enable_if_t<std::is_convertible_v<std::shared_ptr<T>, IControlPtr>>>
        explicit Auto(std::shared_ptr<T> c) : control(std::move(c))
        {
        }
    };

    struct Fixed
    {
        IControlPtr control;
        float value;
        template <typename T, typename = std::enable_if_t<std::is_convertible_v<std::shared_ptr<T>, IControlPtr>>>
        explicit Fixed(std::shared_ptr<T> c, float v) : control(std::move(c)), value(v)
        {
        }
    };

    struct Percent
    {
        IControlPtr control;
        float value;
        template <typename T, typename = std::enable_if_t<std::is_convertible_v<std::shared_ptr<T>, IControlPtr>>>
        explicit Percent(std::shared_ptr<T> c, float v) : control(std::move(c)), value(v)
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
        std::string icon;
        ImVec4 color;
        ImGuiTreeNodeFlags flags;
        PropCat(std::string n, const std::string &i = "", ImVec4 c = ImVec4(0, 0, 0, 0), ImGuiTreeNodeFlags f = ImGuiTreeNodeFlags_DefaultOpen)
            : name(std::move(n)), icon(i), color(c), flags(f)
        {
        }
        // Backwards compatibility constructor
        PropCat(std::string n, bool defaultOpen)
            : name(std::move(n)), icon(""), color({0, 0, 0, 0}), flags(defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0) {}
    };

    struct PropItem
    {
        std::string label;
        IControlPtr editor;
        ImVec4 labelColor;
        template <typename T>
        PropItem(std::string l, std::shared_ptr<T> e, ImVec4 c = ImVec4(0, 0, 0, 0)) : label(std::move(l)), editor(std::move(e)), labelColor(c)
        {
        }
    };

    // --- Property Grid Configuration Tags ---
    struct PGValueColumnWeight
    {
        float weight;
    };
    struct PGItemSpacingY
    {
        float spacing;
    };
    struct PGCategorySpacingY
    {
        float spacing;
    };
    struct PGItemIndent
    {
        float indent;
    };
    struct PGSpanCategoryHeaders
    {
        bool span;
    };
    struct PGZebraStripes
    {
        bool show;
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

    // --- General Control Modifiers ---
    struct SetClass
    {
        std::string cls;
        explicit SetClass(std::string c) : cls(std::move(c)) {}
    };
    struct SetID
    {
        std::string id;
        explicit SetID(std::string i) : id(std::move(i)) {}
    };
    struct Show
    {
    }; // Marker struct for show()
    struct Hide
    {
    }; // Marker struct for hide()
    struct SetVisible
    {
        bool visible;
        explicit SetVisible(bool v) : visible(v) {}
    };
    struct SetEnabled
    {
        bool enabled;
        explicit SetEnabled(bool e) : enabled(e) {}
    };
    struct ReleaseOwnership
    {
    }; // Marker struct
    struct AcquireOwnership
    {
    }; // Marker struct
    struct SetTooltip
    {
        std::string tooltip;
        explicit SetTooltip(std::string t) : tooltip(std::move(t)) {}
    };
    struct SetSpanAvailWidth
    {
        bool span;
        explicit SetSpanAvailWidth(bool s) : span(s) {}
    };
    struct SetUseContainerWidth
    {
        bool use;
        explicit SetUseContainerWidth(bool u) : use(u) {}
    };
    struct SetSizeXY
    {
        float w, h;
        explicit SetSizeXY(float width, float height) : w(width), h(height) {}
    };
    struct SetPositionXY
    {
        float x, y;
        explicit SetPositionXY(float pos_x, float pos_y) : x(pos_x), y(pos_y) {}
    };
    struct SetX
    {
        float x;
        explicit SetX(float pos_x) : x(pos_x) {}
    };
    struct SetY
    {
        float y;
        explicit SetY(float pos_y) : y(pos_y) {}
    };
    struct SetWidth
    {
        float w;
        explicit SetWidth(float width) : w(width) {}
    };
    struct SetHeight
    {
        float h;
        explicit SetHeight(float height) : h(height) {}
    };
    struct AddSize
    {
        float w, h;
        explicit AddSize(float width, float height) : w(width), h(height) {}
    };
    struct ScaleSize
    {
        float factor;
        explicit ScaleSize(float f) : factor(f) {}
    };
    struct ScaleSizeXY
    {
        float factorX, factorY;
        explicit ScaleSizeXY(float fx, float fy) : factorX(fx), factorY(fy) {}
    };
    struct SetMinSize
    {
        float w, h;
        explicit SetMinSize(float width, float height) : w(width), h(height) {}
    };
    struct SetMaxSize
    {
        float w, h;
        explicit SetMaxSize(float width, float height) : w(width), h(height) {}
    };
    struct SetMinWidth
    {
        float w;
        explicit SetMinWidth(float width) : w(width) {}
    };
    struct SetMinHeight
    {
        float h;
        explicit SetMinHeight(float height) : h(height) {}
    };
    struct SetMaxWidth
    {
        float w;
        explicit SetMaxWidth(float width) : w(width) {}
    };
    struct SetMaxHeight
    {
        float h;
        explicit SetMaxHeight(float height) : h(height) {}
    };

    // --- Shadow Modifiers ---
    struct SetShadow
    {
        bool enable;
        ImVec2 offset;
        float blur;
        ImVec4 color;
        float rounding;
        float thickness;
    };
    inline SetShadow MakeSetShadow(bool enable, ImVec2 offset = ImVec2(0, 0), float blur = 10.0f, ImVec4 col = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f), float rounding = -1.0f, float thickness = -1.0f)
    {
        return {enable, offset, blur, col, rounding, thickness};
    }

    struct SetShadowEnabled
    {
        bool enable;
        explicit SetShadowEnabled(bool e) : enable(e) {}
    };
    struct SetShadowOffset
    {
        ImVec2 offset;
        explicit SetShadowOffset(ImVec2 o) : offset(o) {}
    };
    struct SetShadowBlur
    {
        float blur;
        explicit SetShadowBlur(float b) : blur(b) {}
    };
    struct SetShadowColor
    {
        ImVec4 color;
        explicit SetShadowColor(ImVec4 c) : color(c) {}
    };
    struct SetShadowRounding
    {
        float rounding;
        explicit SetShadowRounding(float r) : rounding(r) {}
    };
    struct SetShadowFillBackground
    {
        bool fill;
        explicit SetShadowFillBackground(bool f) : fill(f) {}
    };
    struct SetShadowThickness
    {
        float thickness;
        explicit SetShadowThickness(float t) : thickness(t) {}
    };
    struct DefaultShadow
    {
        bool enable;
        explicit DefaultShadow(bool e = true) : enable(e) {}
    };

    // --- Window Specific Modifiers ---
    struct SetWindowMargined
    {
        bool margined;
        explicit SetWindowMargined(bool m) : margined(m) {}
    };
    struct OnWindowClosing
    {
        std::function<bool()> cb;
        explicit OnWindowClosing(std::function<bool()> c) : cb(std::move(c)) {}
    };
    struct SetNoTabBar
    {
        bool noTabBar;
        explicit SetNoTabBar(bool b) : noTabBar(b) {}
    };
    struct SetTitle
    {
        std::string title;
        explicit SetTitle(std::string t) : title(std::move(t)) {}
    };
    struct SetBorderless
    {
        bool borderless;
        explicit SetBorderless(bool b) : borderless(b) {}
    };
    struct CloseWindow
    {
    }; // Marker struct
    struct FocusWindow
    {
    }; // Marker struct
    struct SetHasMenubar
    {
        bool hasMenubar;
        explicit SetHasMenubar(bool b) : hasMenubar(b) {}
    };
    struct SetWindowResizable
    {
        bool resizable;
        explicit SetWindowResizable(bool b) : resizable(b) {}
    };
    struct SetMovable
    {
        bool movable;
        explicit SetMovable(bool b) : movable(b) {}
    };
    struct SetCollapsible
    {
        bool collapsible;
        explicit SetCollapsible(bool b) : collapsible(b) {}
    };
    struct SetHasTitlebar
    {
        bool hasTitlebar;
        explicit SetHasTitlebar(bool b) : hasTitlebar(b) {}
    };
    struct SetScrollbar
    {
        bool scrollbar;
        explicit SetScrollbar(bool b) : scrollbar(b) {}
    };
    struct SetScrollWithMouse
    {
        bool scrollWithMouse;
        explicit SetScrollWithMouse(bool b) : scrollWithMouse(b) {}
    };
    struct SetAlwaysAutoResize
    {
        bool alwaysAutoResize;
        explicit SetAlwaysAutoResize(bool b) : alwaysAutoResize(b) {}
    };
    struct SetBackground
    {
        bool background;
        explicit SetBackground(bool b) : background(b) {}
    };
    struct SetSavedSettings
    {
        bool savedSettings;
        explicit SetSavedSettings(bool b) : savedSettings(b) {}
    };
    struct SetMouseInputs
    {
        bool mouseInputs;
        explicit SetMouseInputs(bool b) : mouseInputs(b) {}
    };
    struct SetHorizontalScrollbar
    {
        bool horizontalScrollbar;
        explicit SetHorizontalScrollbar(bool b) : horizontalScrollbar(b) {}
    };
    struct SetFocusOnAppearing
    {
        bool focusOnAppearing;
        explicit SetFocusOnAppearing(bool b) : focusOnAppearing(b) {}
    };
    struct SetBringToFrontOnFocus
    {
        bool bringToFrontOnFocus;
        explicit SetBringToFrontOnFocus(bool b) : bringToFrontOnFocus(b) {}
    };
    struct SetAlwaysVerticalScrollbar
    {
        bool alwaysVerticalScrollbar;
        explicit SetAlwaysVerticalScrollbar(bool b) : alwaysVerticalScrollbar(b) {}
    };
    struct SetAlwaysHorizontalScrollbar
    {
        bool alwaysHorizontalScrollbar;
        explicit SetAlwaysHorizontalScrollbar(bool b) : alwaysHorizontalScrollbar(b) {}
    };
    struct SetNavInputs
    {
        bool navInputs;
        explicit SetNavInputs(bool b) : navInputs(b) {}
    };
    // ADDED: Missing Window Specific Modifiers from window.hpp
    struct SetDockID
    {
        Identifier id;
        explicit SetDockID(Identifier i) : id(i) {}
    };
    struct SetNavFocus
    {
        bool navFocus;
        explicit SetNavFocus(bool b) : navFocus(b) {}
    };
    struct SetUnsavedDocument
    {
        bool unsavedDocument;
        explicit SetUnsavedDocument(bool b) : unsavedDocument(b) {}
    };
    struct SetDocking
    {
        bool docking;
        explicit SetDocking(bool b) : docking(b) {}
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
        box->append(child.control, Sizing::Stretch(child.weight));
        return box;
    }

    template <typename BoxType, typename std::enable_if_t<std::is_base_of_v<IControl, typename BoxType::element_type>, int> = 0>
    inline BoxType operator<<(BoxType box, const Auto &child)
    {
        box->append(child.control, Sizing::Auto());
        return box;
    }

    template <typename BoxType, typename std::enable_if_t<std::is_base_of_v<IControl, typename BoxType::element_type>, int> = 0>
    inline BoxType operator<<(BoxType box, const Fixed &child)
    {
        box->append(child.control, Sizing::Fixed(child.value));
        return box;
    }

    template <typename BoxType, typename std::enable_if_t<std::is_base_of_v<IControl, typename BoxType::element_type>, int> = 0>
    inline BoxType operator<<(BoxType box, const Percent &child)
    {
        box->append(child.control, Sizing::Percent(child.value));
        return box;
    }

    // Support pushing a vector of children dynamically into any box layout
    template <typename BoxType, typename std::enable_if_t<std::is_base_of_v<IControl, typename BoxType::element_type>, int> = 0>
    inline BoxType operator<<(BoxType box, const std::vector<IControlPtr> &children)
    {
        for (const auto &child : children)
            box->append(child, false);
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
    inline GridPtr operator<<(GridPtr grid, const std::vector<GridCell> &cells)
    {
        for (const auto &cell : cells)
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
    inline TabPtr operator<<(TabPtr tab, const std::vector<TabPage> &pages)
    {
        for (const auto &page : pages)
            tab << page; // Reuse the single-page operator
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
    inline TablePtr operator<<(TablePtr table, const std::vector<TableRow> &rows)
    {
        for (const auto &row : rows)
            table->addRow(row.items);
        return table;
    }

    // Property Grid
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PropCat &cat)
    {
        pg->addCategory(cat.name, cat.icon, cat.color, cat.flags);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PropItem &item)
    {
        pg->addProperty(item.label, item.editor, item.labelColor);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PGValueColumnWeight &opt)
    {
        pg->setValueColumnWeight(opt.weight);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PGItemSpacingY &opt)
    {
        pg->setItemSpacingY(opt.spacing);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PGCategorySpacingY &opt)
    {
        pg->setCategorySpacingY(opt.spacing);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PGItemIndent &opt)
    {
        pg->setItemIndent(opt.indent);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PGSpanCategoryHeaders &opt)
    {
        pg->setSpanCategoryHeaders(opt.span);
        return pg;
    }
    inline PropertyGridPtr operator<<(PropertyGridPtr pg, const PGZebraStripes &opt)
    {
        pg->setZebraStripes(opt.show);
        return pg;
    }

    // ==========================================
    // 2.2. STREAM OPERATORS (General Control Modifiers)
    // ==========================================
    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetClass &modifier)
    {
        control->addClass(modifier.cls);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetID &modifier)
    {
        control->setID(modifier.id);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const Show &)
    {
        control->show();
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const Hide &)
    {
        control->hide();
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetVisible &modifier)
    {
        control->setVisible(modifier.visible);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetEnabled &modifier)
    {
        control->setEnabled(modifier.enabled);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const ReleaseOwnership &)
    {
        control->releaseOwnership();
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const AcquireOwnership &)
    {
        control->acquireOwnership();
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetTooltip &modifier)
    {
        control->setTooltip(modifier.tooltip);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetSpanAvailWidth &modifier)
    {
        control->setSpanAvailWidth(modifier.span);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetUseContainerWidth &modifier)
    {
        control->setUseContainerWidth(modifier.use);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetSizeXY &modifier)
    {
        control->setSize(modifier.w, modifier.h);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetPositionXY &modifier)
    {
        control->setPosition(modifier.x, modifier.y);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetX &modifier)
    {
        control->setX(modifier.x);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetY &modifier)
    {
        control->setY(modifier.y);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetWidth &modifier)
    {
        control->setWidth(modifier.w);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetHeight &modifier)
    {
        control->setHeight(modifier.h);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const AddSize &modifier)
    {
        control->addSize(modifier.w, modifier.h);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const ScaleSize &modifier)
    {
        control->scaleSize(modifier.factor);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const ScaleSizeXY &modifier)
    {
        control->scaleSize(modifier.factorX, modifier.factorY);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetMinSize &modifier)
    {
        control->setMinSize(modifier.w, modifier.h);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetMaxSize &modifier)
    {
        control->setMaxSize(modifier.w, modifier.h);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetMinWidth &modifier)
    {
        control->setMinWidth(modifier.w);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetMinHeight &modifier)
    {
        control->setMinHeight(modifier.h);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetMaxWidth &modifier)
    {
        control->setMaxWidth(modifier.w);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetMaxHeight &modifier)
    {
        control->setMaxHeight(modifier.h);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadow &modifier)
    {
        control->setShadow(modifier.enable, modifier.offset, modifier.blur, modifier.color, modifier.rounding, modifier.thickness);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadowEnabled &modifier)
    {
        control->setShadowEnabled(modifier.enable);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadowOffset &modifier)
    {
        control->setShadowOffset(modifier.offset);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadowBlur &modifier)
    {
        control->setShadowBlur(modifier.blur);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadowColor &modifier)
    {
        control->setShadowColor(modifier.color);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadowRounding &modifier)
    {
        control->setShadowRounding(modifier.rounding);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadowFillBackground &modifier)
    {
        control->setShadowFillBackground(modifier.fill);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const SetShadowThickness &modifier)
    {
        control->setShadowThickness(modifier.thickness);
        return control;
    }

    template <typename T, typename std::enable_if_t<std::is_base_of_v<IControl, typename T::element_type>, int> = 0>
    inline T operator<<(T control, const DefaultShadow &modifier)
    {
        control->defaultShadow(modifier.enable);
        return control;
    }

    // --- Window Specific Operators ---
    inline WindowPtr operator<<(WindowPtr win, const SetWindowMargined &modifier)
    {
        win->setMargined(modifier.margined);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const OnWindowClosing &modifier)
    {
        win->onClosing(modifier.cb);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetNoTabBar &modifier)
    {
        win->setNoTabBar(modifier.noTabBar);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetTitle &modifier)
    {
        win->setTitle(modifier.title);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetBorderless &modifier)
    {
        win->setBorderless(modifier.borderless);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const CloseWindow &)
    {
        win->close();
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const FocusWindow &)
    {
        win->focus();
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetHasMenubar &modifier)
    {
        win->setHasMenubar(modifier.hasMenubar);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetWindowResizable &modifier)
    {
        win->setResizable(modifier.resizable);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetMovable &modifier)
    {
        win->setMovable(modifier.movable);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetCollapsible &modifier)
    {
        win->setCollapsible(modifier.collapsible);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetHasTitlebar &modifier)
    {
        win->setHasTitlebar(modifier.hasTitlebar);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetScrollbar &modifier)
    {
        win->setScrollbar(modifier.scrollbar);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetScrollWithMouse &modifier)
    {
        win->setScrollWithMouse(modifier.scrollWithMouse);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetAlwaysAutoResize &modifier)
    {
        win->setAlwaysAutoResize(modifier.alwaysAutoResize);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetBackground &modifier)
    {
        win->setBackground(modifier.background);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetSavedSettings &modifier)
    {
        win->setSavedSettings(modifier.savedSettings);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetMouseInputs &modifier)
    {
        win->setMouseInputs(modifier.mouseInputs);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetHorizontalScrollbar &modifier)
    {
        win->setHorizontalScrollbar(modifier.horizontalScrollbar);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetFocusOnAppearing &modifier)
    {
        win->setFocusOnAppearing(modifier.focusOnAppearing);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetBringToFrontOnFocus &modifier)
    {
        win->setBringToFrontOnFocus(modifier.bringToFrontOnFocus);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetAlwaysVerticalScrollbar &modifier)
    {
        win->setAlwaysVerticalScrollbar(modifier.alwaysVerticalScrollbar);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetAlwaysHorizontalScrollbar &modifier)
    {
        win->setAlwaysHorizontalScrollbar(modifier.alwaysHorizontalScrollbar);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetNavInputs &modifier)
    {
        win->setNavInputs(modifier.navInputs);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetDockID &modifier)
    {
        win->setDockId(modifier.id);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetNavFocus &modifier)
    {
        win->setNavFocus(modifier.navFocus);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetUnsavedDocument &modifier)
    {
        win->setUnsavedDocument(modifier.unsavedDocument);
        return win;
    }
    inline WindowPtr operator<<(WindowPtr win, const SetDocking &modifier)
    {
        win->setDocking(modifier.docking);
        return win;
    }

    // Lists & Stacks
    inline ComboBoxPtr operator<<(ComboBoxPtr box, const std::string &item)
    {
        box->append(item);
        return box;
    }
    inline ComboBoxPtr operator<<(ComboBoxPtr box, const std::vector<std::string> &items)
    {
        for (const auto &item : items)
            box->append(item);
        return box;
    }
    inline ListBoxPtr operator<<(ListBoxPtr box, const std::string &item)
    {
        box->append(item);
        return box;
    }
    inline ListBoxPtr operator<<(ListBoxPtr box, const std::vector<std::string> &items)
    {
        for (const auto &item : items)
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
        inline VBoxPtr VBox(bool padded = true, bool scrollable = false, float spacing = -1.0f, bool childScrollbars = false)
        {
            return mui::VBox::create()->setPadded(padded)->setScrollable(scrollable)->setSpacing(spacing)->setShowChildScrollbars(childScrollbars);
        }
        inline HBoxPtr HBox(bool padded = true, bool scrollable = false, float spacing = -1.0f, bool childScrollbars = false)
        {
            return mui::HBox::create()->setPadded(padded)->setScrollable(scrollable)->setSpacing(spacing)->setShowChildScrollbars(childScrollbars);
        }
        inline FlowBoxPtr FlowBox(mui::FlowBox::Align align = mui::FlowBox::Align::Left, bool padded = true, float spacing = -1.0f, bool childScrollbars = false)
        {
            return mui::FlowBox::create()->setAlign(align)->setPadded(padded)->setSpacing(spacing)->setShowChildScrollbars(childScrollbars);
        }
        inline CardPtr Card(float padding = 8.0f, bool fillHeight = false, bool spanAvailWidth = false)
        {
            return mui::Card::create()->setPadding(padding)->setFillHeight(fillHeight)->setSpanAvailWidth(spanAvailWidth);
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

        // --- Advanced Containers ---
        inline std::shared_ptr<mui::PathPicker> PathPicker(mui::PathPickerMode mode = mui::PathPickerMode::File, std::function<void(const std::string &)> onPathChanged = nullptr)
        {
            auto p = mui::PathPicker::create()->setMode(mode);
            if (onPathChanged)
                p->onPathChanged(std::move(onPathChanged));
            return p;
        }
        inline std::shared_ptr<mui::ImageStackView> ImageStackView()
        {
            return mui::ImageStackView::create();
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
        inline SeparatorPtr Spacer(float size = 0.0f)
        {
            return mui::Separator::create()->setType(mui::SeparatorType::Custom)->setThickness(size);
        }
        inline ProgressBarPtr ProgressBar(float value, const std::string &overlay = "")
        {
            return mui::ProgressBar::create()->setValue(value)->setOverlayText(overlay);
        }
        inline ImagePtr Image(const std::string &name, ImTextureID tex, float w = 0, float h = 0)
        {
            return mui::Image::create(name, tex, w, h);
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