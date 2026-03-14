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
        std::map<std::pair<int, int>, ControlPtr> cells;
        int maxRow = 0;
        int maxCol = 0;

    public:
        Grid();
        GridPtr self() { return std::static_pointer_cast<Grid>(shared_from_this()); }

        void render() override;
        GridPtr append(ControlPtr child, int row, int col);
    };
} // namespace mui
