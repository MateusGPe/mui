// widgets/imagestackviewport.hpp
#pragma once
#include "control.hpp"
#include "image.hpp"
#include <vector>
#include <memory>

namespace mui
{
    class ImageStackViewport;
    using ImageStackViewportPtr = std::shared_ptr<ImageStackViewport>;

    class ImageStackView; // Forward declaration

    class ImageStackViewport : public Control<ImageStackViewport>
    {
    protected:
        std::string getTypeName() const override { return "ImageStackViewport"; }
        ImageStackView *m_parent; // Non-owning pointer to parent

        explicit ImageStackViewport(ImageStackView *parent);

    public:
        static ImageStackViewportPtr create(ImageStackView *parent) { return std::shared_ptr<ImageStackViewport>(new ImageStackViewport(parent)); }
        void renderControl() override;
    };
} // namespace mui