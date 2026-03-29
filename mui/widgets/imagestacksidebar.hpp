// widgets/imagestacksidebar.hpp
#pragma once
#include "control.hpp"
#include "image.hpp"
#include "../core/signal.hpp"
#include <vector>
#include <memory>

namespace mui
{
    class ImageStackSidebar;
    using ImageStackSidebarPtr = std::shared_ptr<ImageStackSidebar>;

    class ImageStackView; // Forward declaration

    class ImageStackSidebar : public Control<ImageStackSidebar>
    {
    protected:
        std::string getTypeName() const override { return "ImageStackSidebar"; }
        ImageStackView *m_parent; // Non-owning pointer to parent

        explicit ImageStackSidebar(ImageStackView *parent);

    public:
        // Signals to notify the parent of user actions
        Signal<int> onLayerSelect;
        Signal<int> onLayerMoveUp, onLayerMoveDown, onLayerRemove;
        Signal<> onFitView;
        Signal<bool> onToggleThumbnailMode, onToggleSingleLayerMode, onToggleSlideshowMode;

        static ImageStackSidebarPtr create(ImageStackView *parent) { return std::shared_ptr<ImageStackSidebar>(new ImageStackSidebar(parent)); }
        void renderControl() override;
    };
} // namespace mui