// widgets/breadcrumb.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mui
{
    class BreadcrumbBar;
    using BreadcrumbBarPtr = std::shared_ptr<BreadcrumbBar>;

    class BreadcrumbBar : public Control<BreadcrumbBar>
    {
    protected:
        std::string currentPath;
        std::vector<std::string> segments;
        bool isEditing = false;
        char editBuffer[1024] = "";

        std::function<void(const std::string&)> onPathNavigatedCb;

        void parsePath();

    public:
        BreadcrumbBar(const std::string& path = "");
        static BreadcrumbBarPtr create(const std::string& path = "") { return std::make_shared<BreadcrumbBar>(path); }

        void renderControl() override;

        std::string getPath() const;
        BreadcrumbBarPtr setPath(const std::string& path);
        BreadcrumbBarPtr onPathNavigated(std::function<void(const std::string&)> cb);
    };
} // namespace mui
