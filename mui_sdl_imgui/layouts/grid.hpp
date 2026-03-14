#pragma once
#include "control.hpp"
#include <map>
#include <utility>

namespace mui
{
    class Grid;
    using GridPtr = std::shared_ptr<Grid>;

    class Grid : public Control
    {
        struct Cell
        {
            ControlPtr control;
            int colSpan = 1;
        };
        std::map<std::pair<int, int>, Cell> cells;
        int maxRow = 0;
        int maxCol = 0;

    public:
        Grid();
        static GridPtr create() { return std::make_shared<Grid>(); }
        GridPtr self() { return std::static_pointer_cast<Grid>(shared_from_this()); }

        void render() override;
        GridPtr append(ControlPtr child, int row, int col, int colSpan = 1);
    };
} // namespace mui
