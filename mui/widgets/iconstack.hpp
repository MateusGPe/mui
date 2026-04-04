// widgets/iconstack.hpp
#pragma once
#include "control.hpp"
#include <vector>
#include <string>
#include <functional> // <-- Added this!
#include "../core/signal.hpp"

namespace mui
{
    class IconStack;
    using IconStackPtr = std::shared_ptr<IconStack>;

    class IconStack : public Control<IconStack>
    {
    protected:
        struct IconBtn
        {
            std::string icon;
            std::string tooltip;
            bool text_hover;
            float cached_width = -1.0f;
        };
        std::vector<IconBtn> icons;
        
        // New constraint and scaling properties
        ImVec2 m_minIconSize = ImVec2(0.0f, 0.0f);
        ImVec2 m_maxIconSize = ImVec2(FLT_MAX, FLT_MAX);
        bool m_scaleToFit = false;

        IconStack();

    public:
        mui::Signal<int> onIconClickedSignal;

        static IconStackPtr create() { return std::shared_ptr<IconStack>(new IconStack()); }
        std::string getTypeName() const override { return "IconStack"; }

        void renderControl() override;
        IconStackPtr add(const std::string &icon, std::function<void()> onClick, const std::string &tip = "", bool textHover = false);

        // --- Chainable Setters for new features ---
        IconStackPtr minIconSize(const ImVec2& size) { m_minIconSize = size; return self(); }
        IconStackPtr maxIconSize(const ImVec2& size) { m_maxIconSize = size; return self(); }
        IconStackPtr scaleToFit(bool scale = true) { m_scaleToFit = scale; return self(); }
    };
}
