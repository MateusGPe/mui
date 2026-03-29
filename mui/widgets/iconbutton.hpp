// widgets/iconbutton.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <functional>
#include <memory>
#include "../core/signal.hpp"
#include <imgui.h>

namespace mui
{
    enum class IconButtonLayout
    {
        Horizontal,
        Vertical
    };

    class IconButton;
    using IconButtonPtr = std::shared_ptr<IconButton>;

    class IconButton : public Control<IconButton>
    {
    protected:
        std::string getTypeName() const override { return "IconButton"; }
        std::string text;
        std::string iconText;
        ImTextureID iconTex = 0;
        IconButtonLayout layout = IconButtonLayout::Horizontal;

        bool selected = false;
        ImVec2 iconSize = ImVec2(16.0f, 16.0f);

        explicit IconButton(const std::string &text = "");

    public:
        mui::Signal<> onClickSignal;
        mui::Signal<> onDoubleClickSignal;

        static IconButtonPtr create(const std::string &text = "") { return std::shared_ptr<IconButton>(new IconButton(text)); }

        void renderControl() override;

        IconButtonPtr setText(const std::string &t);
        IconButtonPtr setIconText(const std::string &t);
        IconButtonPtr setIconTexture(ImTextureID tex);
        IconButtonPtr setLayout(IconButtonLayout l);
        IconButtonPtr setSelected(bool s);
        IconButtonPtr setIconSize(float w, float h);
        IconButtonPtr setUseContainerWidth(bool use);

        IconButtonPtr onClick(std::function<void()> cb);
        IconButtonPtr onDoubleClick(std::function<void()> cb);
    };
} // namespace mui
