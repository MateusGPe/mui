// widgets/table.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

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
        bool sortable = false;

        std::function<void(int)> onRowSelectedCb;
        std::function<void(int)> onRowDoubleClickedCb;
        std::function<void(int, bool)> onSortRequestedCb; // Column index, Ascending

    public:
        Table();
        static TablePtr create() { return std::make_shared<Table>(); }

        void renderControl() override;

        TablePtr addColumn(const std::string &name, float weight = 1.0f, bool fixedWidth = false);
        TablePtr addRow(const std::vector<IControlPtr> &rowItems);
        TablePtr clearRows();

        int getSelectedRow() const;
        TablePtr setSelectedRow(int index);
        TablePtr setSortable(bool s);

        TablePtr onRowSelected(std::function<void(int)> cb);
        TablePtr onRowDoubleClicked(std::function<void(int)> cb);
        TablePtr onSortRequested(std::function<void(int, bool)> cb);
    };
} // namespace mui
