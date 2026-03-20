#pragma once

#include <imgui.h>
#include <string>
#include <imgui_internal.h> // For ImGuiDockNode
namespace mui
{
    class DockNode; // Forward declaration

    enum class DockDirection
    {
        Left = ImGuiDir_Left,
        Right = ImGuiDir_Right,
        Up = ImGuiDir_Up,
        Down = ImGuiDir_Down,
    };

    class DockBuilder
    {
    public:
        DockBuilder(ImGuiID dockspaceId);

        ImGuiID getID() const { return m_dockspaceId; }

        ImGuiID splitNode(ImGuiID &nodeIdToSplit, DockDirection dir, float ratio);

        DockNode getNode(ImGuiID nodeId);

        void dockWindow(const char *windowName, ImGuiID nodeId);
        void dockWindow(const std::string &windowName, ImGuiID nodeId);

    private:
        ImGuiID m_dockspaceId;
    };

    class DockNode
    {
    public:
        DockNode(ImGuiDockNode *node);
        void setNoTabBar(bool noTabBar);
        bool isValid() const { return m_node != nullptr; }
    private:
        ImGuiDockNode *m_node;
    };
} // namespace mui