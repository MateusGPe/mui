// widgets/imagestackview.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imagestackview.hpp"
#include "../core/app.hpp"
#include "../core/scoped.hpp"
#include "../include/IconsFontAwesome7.h"

#ifndef ICON_FA_EYE
#define ICON_FA_EYE "\xef\x81\xae"
#endif
#ifndef ICON_FA_EYE_SLASH
#define ICON_FA_EYE_SLASH "\xef\x81\xb0"
#endif
#ifndef ICON_FA_ARROW_UP
#define ICON_FA_ARROW_UP "\xef\x81\xa2"
#endif
#ifndef ICON_FA_ARROW_DOWN
#define ICON_FA_ARROW_DOWN "\xef\x81\xa3"
#endif
#ifndef ICON_FA_TRASH
#define ICON_FA_TRASH "\xef\x81\x93"
#endif
#ifndef ICON_FA_PLAY
#define ICON_FA_PLAY "\xef\x81\x8b"
#endif
#ifndef ICON_FA_PAUSE
#define ICON_FA_PAUSE "\xef\x81\x8c"
#endif

namespace mui
{

    ImageStackView::ImageStackView()
    {
        App::assertMainThread();
        m_viewport = ImageStackViewport::create(this);
        m_sidebar = ImageStackSidebar::create(this);

        // Connect signals from sidebar to this view's methods
        addConnection(m_sidebar->onLayerSelect.connect([this](int index)
                                                                 {
    m_selectedLayer = index;
    m_slideshowTimer = 0.0f; }));
        addConnection(m_sidebar->onLayerMoveUp.connect(
            [this](int index)
            { moveLayerUp(index); }));
        addConnection(m_sidebar->onLayerMoveDown.connect(
            [this](int index)
            { moveLayerDown(index); }));
        addConnection(m_sidebar->onLayerRemove.connect(
            [this](int index)
            { removeLayer(index); }));
        addConnection(
            m_sidebar->onFitView.connect([this]()
                                         { fitToView(); }));
        addConnection(m_sidebar->onToggleThumbnailMode.connect(
            [this](bool mode)
            { setThumbnailMode(mode); }));
        addConnection(m_sidebar->onToggleSingleLayerMode.connect(
            [this](bool mode)
            { setSingleLayerMode(mode); }));
        addConnection(m_sidebar->onToggleSlideshowMode.connect(
            [this](bool mode)
            { setSlideshowMode(mode); }));
    }

    ImageStackView::~ImageStackView() { clearLayers(); }

    ImageStackViewPtr ImageStackView::addLayer(const ImagePtr& image)
    {
        if (!image)
            return self();
        m_layers.push_back(image);
        if (m_firstRender && m_layers.size() == 1)
        {
            fitToView();
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::loadFromMemory(const std::string &name,
                                                     int width, int height,
                                                     int channels,
                                                     const unsigned char *data)
    {
        auto image = Image::loadFromMemory(name, width, height, channels, data);
        if (image)
        {
            addLayer(image);
        }

        return self();
    }

    ImageStackViewPtr ImageStackView::removeLayer(int index)
    {
        if (index >= 0 && index < (int)m_layers.size())
        {
            m_layers.erase(m_layers.begin() + index);
            if (m_selectedLayer >= (int)m_layers.size())
            {
                m_selectedLayer = std::max(0, (int)m_layers.size() - 1);
            }
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::moveLayerUp(int index)
    {
        if (index >= 0 && index < (int)m_layers.size() - 1)
        {
            std::swap(m_layers[index], m_layers[index + 1]);
            if (m_selectedLayer == index)
                m_selectedLayer++;
            else if (m_selectedLayer == index + 1)
                m_selectedLayer--;
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::moveLayerDown(int index)
    {
        if (index > 0 && index < (int)m_layers.size())
        {
            std::swap(m_layers[index], m_layers[index - 1]);
            if (m_selectedLayer == index)
                m_selectedLayer--;
            else if (m_selectedLayer == index - 1)
                m_selectedLayer++;
        }
        return self();
    }

    ImageStackViewPtr ImageStackView::clearLayers()
    {
        m_layers.clear();
        m_selectedLayer = 0;
        return self();
    }

    ImageStackViewPtr ImageStackView::fitToView()
    {
        m_firstRender = true;
        return self();
    }

    ImageStackViewPtr ImageStackView::resetZoom()
    {
        m_scale = 1.0f;
        m_translate = ImVec2(0, 0);
        m_firstRender = true;
        return self();
    }

    ImageStackViewPtr ImageStackView::setSidebarWidth(float width)
    {
        m_sidebarWidth = width;
        return self();
    }

    ImageStackViewPtr ImageStackView::setSingleLayerMode(bool single)
    {
        m_singleLayerMode = single;
        m_firstRender = true;
        return self();
    }

    ImageStackViewPtr ImageStackView::setThumbnailMode(bool thumb)
    {
        m_thumbnailMode = thumb;
        return self();
    }

    ImageStackViewPtr ImageStackView::setSlideshowMode(bool slideshow,
                                                       float interval)
    {
        m_slideshowMode = slideshow;
        m_slideshowInterval = interval;
        m_slideshowTimer = 0.0f;
        if (m_slideshowMode && !m_singleLayerMode)
        {
            m_singleLayerMode = true; // Auto-switch to single mode for slideshow
        }
        return self();
    }

    void ImageStackView::renderControl()
    {
        if (!visible)
            return;
        ScopedControlID id(this);
        ImGui::BeginDisabled(!enabled);

        // Auto-advance slideshow timer
        if (m_slideshowMode && !m_layers.empty())
        {
            m_slideshowTimer += ImGui::GetIO().DeltaTime;
            if (m_slideshowTimer >= m_slideshowInterval)
            {
                m_slideshowTimer = 0.0f;
                m_selectedLayer--; // Decrement traverses top visually, simulating
                                   // chronological
                if (m_selectedLayer < 0)
                    m_selectedLayer = (int)m_layers.size() - 1;
                m_firstRender = true; // Auto-fit new sizes
            }
        }

        float actual_sidebar_w = m_sidebarWidth * App::getDpiScale();
        float avail_w = ImGui::GetContentRegionAvail().x;
        float avail_h = ImGui::GetContentRegionAvail().y;

        // Calculate canvas width, ensuring it never goes negative
        float canvas_w = std::max(1.0f, avail_w - actual_sidebar_w -
                                            ImGui::GetStyle().ItemSpacing.x);

        // Render Viewport (Canvas)
        // Push 0 padding so the canvas draws edge-to-edge
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::BeginChild("##imagestack_viewport", ImVec2(canvas_w, avail_h), false,
                          ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse);
        m_viewport->renderControl();
        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::SameLine();

        // Render Sidebar
        ImGui::BeginChild("##imagestack_sidebar", ImVec2(actual_sidebar_w, avail_h),
                          false);
        m_sidebar->renderControl();
        ImGui::EndChild();

        renderTooltip();
        ImGui::EndDisabled();
    }

} // namespace mui