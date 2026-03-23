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
    protected:
        struct Cell
        {
            IControlPtr control;
            int colSpan = 1;
        };
        std::map<std::pair<int, int>, Cell> m_cells;
        std::map<int, float> m_columnWeights;
        int m_maxRow = 0;
        int m_maxCol = 0;

        Grid();

    public:
        static GridPtr create() { return std::shared_ptr<Grid>(new Grid()); }

        GridPtr setColumnWeight(int col, float weight);
        void renderControl() override;
        GridPtr append(IControlPtr child, int row, int col, int colSpan = 1);
    };
} // namespace mui
