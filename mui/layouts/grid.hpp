#pragma once
#include "../widgets/control.hpp"
#include <map>
#include <utility>

namespace mui
{
    class Grid;
    using GridPtr = std::shared_ptr<Grid>;

    class Grid : public Control<Grid>
    {
        struct Cell
        {
            IControlPtr control;
            int colSpan = 1;
        };
        std::map<std::pair<int, int>, Cell> m_cells;
        std::map<int, float> m_columnWeights;
        int m_maxRow = 0;
        int m_maxCol = 0;

    public:
        Grid();
        static GridPtr create() { return std::make_shared<Grid>(); }

        GridPtr setColumnWeight(int col, float weight);
        void renderControl() override;
        GridPtr append(IControlPtr child, int row, int col, int colSpan = 1);
    };
} // namespace mui
