// widgets/imagestackview.hpp
#pragma once
#include "control.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <imgui.h>

namespace mui
{
    struct ImageLayer {
        std::string name;
        ImTextureID textureId;
        float width;
        float height;
        bool visible = true;
        bool ownsTexture = false;
        float opacity = 1.0f;
    };

    class ImageStackView;
    using ImageStackViewPtr = std::shared_ptr<ImageStackView>;

    class ImageStackView : public Control<ImageStackView>
    {
    protected:
        std::string getTypeName() const override { return "ImageStackView"; }
        
        std::vector<ImageLayer> m_layers;
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

        void updateZoom(ImVec2 canvasOrigin, ImVec2 canvasSize);
        void renderSidebar(ImVec2 size);
        void freeTexture(ImTextureID tex);

        explicit ImageStackView();

    public:
        ~ImageStackView() override;

        static ImageStackViewPtr create() { return std::shared_ptr<ImageStackView>(new ImageStackView()); }

        void renderControl() override;
        
        // Layer Management
        ImageStackViewPtr addLayer(const std::string& name, ImTextureID tex, float w, float h, bool takeOwnership = false);
        ImageStackViewPtr loadFromMemory(const std::string& name, int width, int height, int channels, const unsigned char* data);
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