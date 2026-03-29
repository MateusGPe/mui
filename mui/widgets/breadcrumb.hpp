// widgets/breadcrumb.hpp
#pragma once
#include "control.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <imgui.h>
#include "entry.hpp"

namespace mui
{
    class BreadcrumbBar;
    using BreadcrumbBarPtr = std::shared_ptr<BreadcrumbBar>;

    class BreadcrumbBar : public Control<BreadcrumbBar>
    {
    protected:
        std::string getTypeName() const override { return "BreadcrumbBar"; }
        std::string currentPath;
        std::vector<std::string> segments;
        char editBuffer[1024] = {0};
        bool isEditing = false;
        EntryPtr m_editEntry;

        void parsePath();

        explicit BreadcrumbBar(const std::string &path = "");

    public:
        mui::Signal<std::string> onPathNavigatedSignal;

        static BreadcrumbBarPtr create(const std::string &path = "") { return std::shared_ptr<BreadcrumbBar>(new BreadcrumbBar(path)); }

        void renderControl() override;

        std::string getPath() const;
        BreadcrumbBarPtr setPath(const std::string &path);
        BreadcrumbBarPtr bind(std::shared_ptr<Observable<std::string>> observable);
        BreadcrumbBarPtr onPathNavigated(std::function<void(const std::string &)> cb);

        bool getIsEditing() const { return isEditing; }
        BreadcrumbBarPtr setIsEditing(bool editing);
    };
} // namespace mui
