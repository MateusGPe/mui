// widgets/iconstack.hpp
#pragma once
#include "control.hpp"
#include <vector>
#include <string>
#include <functional> // <-- Added this!

namespace mui
{
    class IconStack;
    using IconStackPtr = std::shared_ptr<IconStack>;

    class IconStack : public Control
    {
    protected:
        struct IconBtn {
            std::string icon;
            std::string tooltip;
            std::function<void()> cb;
        };
        std::vector<IconBtn> icons;

        IconStackPtr self() { return std::static_pointer_cast<IconStack>(shared_from_this()); }

    public:
        IconStack();
        static IconStackPtr create() { return std::make_shared<IconStack>(); }

        void renderControl() override;
        IconStackPtr add(const std::string& icon, std::function<void()> onClick, const std::string& tip = "");
    };
}
