// widgets/propertygrid.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <memory>

namespace mui
{
    class PropertyGrid;
    using PropertyGridPtr = std::shared_ptr<PropertyGrid>;

    class PropertyGrid : public Control<PropertyGrid>
    {
    protected:
        std::string getTypeName() const override { return "PropertyGrid"; }
        struct PropertyItem
        {
            std::string label;
            IControlPtr editor;
        };

        struct PropertyCategory
        {
            std::string name;
            std::vector<PropertyItem> items;
            bool defaultOpen = true;
        };

        std::vector<PropertyCategory> categories;
        float nameColumnWidth = 150.0f;
        PropertyGrid();

    public:
        static PropertyGridPtr create() { return std::shared_ptr<PropertyGrid>(new PropertyGrid()); }

        void renderControl() override;

        // Adds a new collapsing category header.
        PropertyGridPtr addCategory(const std::string &name, bool defaultOpen = true);

        // Adds a property to the *most recently added* category.
        // If no category was added yet, it creates a flat/hidden category automatically.
        PropertyGridPtr addProperty(const std::string &label, IControlPtr editor);

        // Set the default width of the property name column (the left column).
        PropertyGridPtr setNameColumnWidth(float width);
    };
} // namespace mui