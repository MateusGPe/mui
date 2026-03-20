// widgets/breadcrumb.cpp
#include "breadcrumb.hpp"
#include "../core/app.hpp"
#include <imgui.h>
#include <cstring>
#include <filesystem>

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
        std::filesystem::path p(currentPath);
        for (auto it = p.begin(); it != p.end(); ++it)
        {
            std::string segment = it->u8string();
            // Ignore trailing/duplicate slashes
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
        ImGui::PushID(this);

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("##breadcrumb_host", ImVec2(spanAvailWidth ? avail.x : 0, ImGui::GetFrameHeight()), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        if (isEditing)
        {
            ImGui::SetKeyboardFocusHere();
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::InputText("##pathedit", editBuffer, sizeof(editBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                setPath(editBuffer);
                isEditing = false;
                if (onPathNavigatedCb)
                    onPathNavigatedCb(currentPath);
            }
            ImGui::PopItemWidth();

            // Exit edit mode if user clicks away
            if (!ImGui::IsItemActive() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
            {
                isEditing = false;
            }
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent buttons
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

            for (size_t i = 0; i < segments.size(); ++i)
            {
                // Prevent ImGui assertion by skipping empty segments.
                if (segments[i].empty()) continue;

                if (ImGui::Button(segments[i].c_str()))
                {
                    // Reconstruct path up to this segment
                    std::string newPath = "";
                    for (size_t j = 0; j <= i; ++j)
                    {
                        newPath += segments[j];
                        if (j < i || i == 0)
                            newPath += "/";
                    }
                    setPath(newPath);
                    if (onPathNavigatedCb)
                        onPathNavigatedCb(currentPath);
                }

                if (i < segments.size() - 1)
                {
                    ImGui::SameLine();
                    ImGui::TextDisabled(">");
                    ImGui::SameLine();
                }
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            // Clicking empty space switches to edit mode
            ImGui::SameLine();
            ImVec2 remainingSpace = ImGui::GetContentRegionAvail();
            if (remainingSpace.x > 10.0f)
            {
                if (ImGui::InvisibleButton("##empty_space", remainingSpace))
                {
                    isEditing = true;
                    std::strncpy(editBuffer, currentPath.c_str(), sizeof(editBuffer) - 1);
                }
            }
        }

        ImGui::EndChild();
        ImGui::PopID();
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
} // namespace mui
