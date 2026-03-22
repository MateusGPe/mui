// widgets/table.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../core/signal.hpp"
#include "../core/observable.hpp"

namespace mui
{
    class Table;
    using TablePtr = std::shared_ptr<Table>;

    struct TableColumn
    {
        std::string name;
        float weight = 1.0f;
        bool fixedWidth = false;
    };

    class Table : public Control<Table>
    {
    protected:
        std::vector<TableColumn> columns;
        std::vector<std::vector<IControlPtr>> rows;
        int selectedRow = -1;
        ImGuiTableFlags m_flags;
    
    public:
        mui::Signal<int> onRowSelectedSignal;
        mui::Signal<int> onRowDoubleClickedSignal;
        mui::Signal<int, bool> onSortRequestedSignal; // Column index, Ascending
    
        Table();
        static TablePtr create() { return std::make_shared<Table>(); }
    
        void renderControl() override;
    
        TablePtr addColumn(const std::string &name, float weight = 1.0f, bool fixedWidth = false);
        TablePtr addRow(const std::vector<IControlPtr> &rowItems);
        TablePtr clearRows();
    
        int getSelectedRow() const;
        TablePtr setSelectedRow(int index);
        TablePtr bind(std::shared_ptr<Observable<int>> observable);
        TablePtr setSortable(bool s);
    
        TablePtr setResizable(bool b);
        TablePtr setReorderable(bool b);
        TablePtr setHideable(bool b);
        TablePtr setNoSavedSettings(bool b);
        TablePtr setContextMenuInBody(bool b);
        TablePtr setScrollX(bool b);
    
        TablePtr onRowSelected(std::function<void(int)> cb);
        TablePtr onRowDoubleClicked(std::function<void(int)> cb);
        TablePtr onSortRequested(std::function<void(int, bool)> cb);
    };} // namespace mui
