// widgets/imagestackview.hpp
#pragma once
#include "control.hpp"
#include "image.hpp"
#include "imagestackviewport.hpp"
#include "imagestacksidebar.hpp"
// REMOVED: #include "../layouts/box.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <imgui.h>

namespace mui
{
    class ImageStackView;
    using ImageStackViewPtr = std::shared_ptr<ImageStackView>;

    class ImageStackView : public Control<ImageStackView>
    {
        friend class ImageStackViewport;
        friend class ImageStackSidebar;

    protected:
        std::string getTypeName() const override { return "ImageStackView"; }

        std::vector<ImagePtr> m_layers;
        int m_selectedLayer = 0;

        // Transform State
        ImVec2 m_translate = {0, 0};
        float m_scale = 1.0f;
        bool m_firstRender = true;
        float m_sidebarWidth = 200.0f;

        bool m_singleLayerMode = false;
        bool m_thumbnailMode = false;
        bool m_slideshowMode = false;
        float m_slideshowInterval = 2.0f;
        float m_slideshowTimer = 0.0f;

        ImageStackViewportPtr m_viewport;
        ImageStackSidebarPtr m_sidebar;

        // REMOVED: HBoxPtr m_layout;
        explicit ImageStackView();

    public:
        ~ImageStackView() override;

        static ImageStackViewPtr create() { return std::shared_ptr<ImageStackView>(new ImageStackView()); }

        void renderControl() override;

        // Layer Management
        ImageStackViewPtr addLayer(const ImagePtr& image);
        ImageStackViewPtr loadFromMemory(const std::string &name, int width, int height, int channels, const unsigned char *data);
        ImageStackViewPtr removeLayer(int index);
        ImageStackViewPtr moveLayerUp(int index);
        ImageStackViewPtr moveLayerDown(int index);
        ImageStackViewPtr clearLayers();

        // View Controls
        ImageStackViewPtr fitToView();
        ImageStackViewPtr resetZoom();
        ImageStackViewPtr setSidebarWidth(float width);

        ImageStackViewPtr setSingleLayerMode(bool single);
        ImageStackViewPtr setThumbnailMode(bool thumb);
        ImageStackViewPtr setSlideshowMode(bool slideshow, float interval = 2.0f);
    };
} // namespace mui