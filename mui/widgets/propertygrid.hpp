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

    enum PropertyGridFlags_
    {
        PropertyGridFlags_None = 0,
        PropertyGridFlags_Visible = 1 << 0,
        PropertyGridFlags_Enabled = 1 << 1,
        PropertyGridFlags_ZebraStripes = 1 << 2,
        PropertyGridFlags_SpanCategoryHeaders = 1 << 3,

        PropertyGridFlags_Default = PropertyGridFlags_Visible | PropertyGridFlags_Enabled | PropertyGridFlags_ZebraStripes
    };

    class PropertyGrid : public Control<PropertyGrid>
    {
    protected:
        std::string getTypeName() const override { return "PropertyGrid"; }
        struct PropertyItem
        {
            std::string label;
            ImVec4 labelColor;          // New: Custom color for the label
            bool useLabelColor = false; // New: Flag to indicate if custom color is used
            IControlPtr editor;
        };

        struct PropertyCategory
        {
            std::string name;
            std::string icon;            // New: Icon for the category header
            ImVec4 color;                // New: Custom color for the category header
            bool useCustomColor = false; // New: Flag to indicate if custom color is used
            std::vector<PropertyItem> items;
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen; // New: ImGuiTreeNodeFlags for the header
        };

        std::vector<PropertyCategory> categories;
        float nameColumnWidth = 150.0f;
        float valueColumnWeight = 1.0f; // New: Weight for the value column in stretch mode
        float itemSpacingY = -1.0f;     // New: Vertical spacing between items (-1.0f for default)
        float categorySpacingY = -1.0f; // New: Vertical spacing between categories (-1.0f for default)
        float itemIndent = -1.0f;       // New: Indentation for items under categories (-1.0f for default)
        uint32_t m_flags = PropertyGridFlags_Default;
        PropertyGrid();

    public:
        static PropertyGridPtr create() { return std::shared_ptr<PropertyGrid>(new PropertyGrid()); }

        void renderControl() override;

        // Adds a new collapsing category header.
        // New parameters: icon, color, and ImGuiTreeNodeFlags for more customization.
        PropertyGridPtr addCategory(const std::string &name, const std::string &icon = "", ImVec4 color = ImVec4(0, 0, 0, 0), ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen);

        // Adds a property to the *most recently added* category.
        // If no category was added yet, it creates a flat/hidden category automatically.
        // New parameter: labelColor for custom label styling.
        PropertyGridPtr addProperty(const std::string &label, IControlPtr editor, ImVec4 labelColor = ImVec4(0, 0, 0, 0));

        // Set the default width of the property name column (the left column).
        PropertyGridPtr setNameColumnWidth(float width);

        // New: Set the weight for the value column when using SizingStretchProp.
        PropertyGridPtr setValueColumnWeight(float weight);
        // New: Set custom vertical spacing between property items.
        PropertyGridPtr setItemSpacingY(float spacing);
        // New: Set custom vertical spacing between categories.
        PropertyGridPtr setCategorySpacingY(float spacing);
        // New: Set custom indentation for items within categories.
        PropertyGridPtr setItemIndent(float indent);

        // Visibility and Enabled overrides to update m_flags
        PropertyGridPtr setVisible(bool v);
        PropertyGridPtr setEnabled(bool e);

        // If true, alternating row backgrounds (zebra stripes) will be drawn.
        PropertyGridPtr setZebraStripes(bool show);

        // If true, category headers will span all columns of the grid.
        PropertyGridPtr setSpanCategoryHeaders(bool span);
    };
} // namespace mui