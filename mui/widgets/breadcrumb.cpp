// widgets/breadcrumb.cpp
#include "breadcrumb.hpp"
#include "iconbutton.hpp"
#include "../core/app.hpp"
#include "IconsFontAwesome6.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "../core/scoped.hpp"
#include <cstring>
#include <filesystem>

#ifndef ICON_FA_ANGLE_RIGHT
#define ICON_FA_ANGLE_RIGHT "\xef\x84\x85"
#endif

namespace mui
{
    BreadcrumbBar::BreadcrumbBar(const std::string &path)
    {
        App::assertMainThread();
        currentPath = path;
        parsePath();
    }

    void BreadcrumbBar::parsePath()
    {
        segments.clear();
        std::filesystem::path p(std::filesystem::u8path(currentPath));
        for (auto it = p.begin(); it != p.end(); ++it)
        {
            std::string segment = it->u8string();
            if (segment != "\\" && segment != "/")
            {
                segments.push_back(segment);
            }
        }
    }

    void BreadcrumbBar::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);
        ImGui::BeginDisabled(!enabled);

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        ImGuiStyle &style = ImGui::GetStyle();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 actualSize(spanAvailWidth ? avail.x : width, height > 0 ? height : ImGui::GetFrameHeight());
        
        if (actualSize.x <= 0.0f) actualSize.x = avail.x; 

        // Get absolute position to calculate hover over the child rect
        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos, ImVec2(pos.x + actualSize.x, pos.y + actualSize.y));

        bool hovered = ImGui::IsMouseHoveringRect(bb.Min, bb.Max);
        bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);

        // Use Child Window to perfectly encapsulate the cursor changes and clipping
        ScopedColor childBg(ImGuiCol_ChildBg, style.Colors[hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]);
        ScopedStyle styleVars;
        styleVars.push(ImGuiStyleVar_ChildRounding, style.FrameRounding);
        styleVars.push(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::BeginChild("##bc_host", actualSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        if (!isEditing)
        {
            ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, style.ItemSpacing.y));
            bool anyItemInteraction = false;

            for (size_t i = 0; i < segments.size(); ++i)
            {
                if (segments[i].empty())
                    continue;

                ScopedID segment_id(static_cast<int>(i));

                if (i > 0)
                {
                    {
                        ScopedID sep_id("sep");
                        auto sep = mui::IconButton::create("")
                                       ->setIconText(ICON_FA_ANGLE_RIGHT)
                                       ->setSize(ImGui::GetFrameHeight(), actualSize.y);
                        sep->render();
                        anyItemInteraction |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
                    }
                    ImGui::SameLine();
                }

                bool btnClicked = false;
                {
                    ScopedID btn_id("btn");
                    auto btn = mui::IconButton::create(segments[i])
                                   ->setLayout(mui::IconButtonLayout::Horizontal)
                                   ->setSize(0, actualSize.y)
                               ->onClick([&]()
                                         { btnClicked = true; });
                    btn->render();
                    anyItemInteraction |= ImGui::IsItemHovered() | btnClicked;
                }

                if (btnClicked)
                {
                    std::string newPath = "";
#ifndef _WIN32
                    // CRITICAL FIX: Ensure Unix platforms restore the absolute path root `/`
                    if (!segments.empty() && segments[0] != "This PC" && segments[0] != "Quick Access")
                        newPath = "/";
#endif
                    for (size_t j = 0; j <= i; ++j)
                    {
                        newPath += segments[j];
#ifdef _WIN32
                        if (j != i)
                            newPath += "\\";
#else
                        if (j != i)
                            newPath += "/";
#endif
                    }
                    setPath(newPath);
                    if (onPathNavigatedCb)
                        onPathNavigatedCb(currentPath);
                }
                ImGui::SameLine();
            }

            if (!anyItemInteraction && clicked)
            {
                isEditing = true;
                std::strncpy(editBuffer, currentPath.c_str(), sizeof(editBuffer) - 1);
            }
        }
        else
        {
            ImGui::SetKeyboardFocusHere();

            float expectedPaddingY = (actualSize.y - ImGui::GetFontSize()) / 2.0f;
            ScopedStyle framePadding;
            if (expectedPaddingY > style.FramePadding.y)
            {
                framePadding.push(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, expectedPaddingY));
            }

            ScopedItemWidth width(-FLT_MIN);
            if (ImGui::InputTextEx("##bc_edit", "", editBuffer, sizeof(editBuffer), ImVec2(-FLT_MIN, actualSize.y), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                setPath(editBuffer);
                isEditing = false;
                if (onPathNavigatedCb)
                    onPathNavigatedCb(currentPath);
            }

            if (!ImGui::IsItemActive() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
            {
                isEditing = false;
            }
        }

        ImGui::EndChild();

        renderTooltip();
        ImGui::EndDisabled();
    }

    std::string BreadcrumbBar::getPath() const { return currentPath; }

    BreadcrumbBarPtr BreadcrumbBar::setPath(const std::string &path)
    {
        currentPath = path;
        parsePath();
        return self();
    }

    BreadcrumbBarPtr BreadcrumbBar::onPathNavigated(std::function<void(const std::string &)> cb)
    {
        onPathNavigatedCb = std::move(cb);
        return self();
    }

    BreadcrumbBarPtr BreadcrumbBar::setSize(float w, float h)
    {
        width = w;
        height = h;
        return self();
    }

    BreadcrumbBarPtr BreadcrumbBar::setIsEditing(bool editing)
    {
        isEditing = editing;
        if (editing)
            std::strncpy(editBuffer, currentPath.c_str(), sizeof(editBuffer) - 1);
        return self();
    }
} // namespace mui
