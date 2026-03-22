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

    public:
        PropertyGrid();
        static PropertyGridPtr create() { return std::make_shared<PropertyGrid>(); }

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