#include "docking.hpp"
#include <imgui_internal.h> // For ImGuiDockNode

namespace mui
{
    // --- DockNode ---
    DockNode::DockNode(ImGuiDockNode *node) : m_node(node) {}

    void DockNode::setNoTabBar(bool noTabBar)
    {
        if (m_node)
        {
            if (noTabBar)
            {
                m_node->SetLocalFlags(m_node->LocalFlags | ImGuiDockNodeFlags_NoTabBar);
            }
            else
            {
                m_node->SetLocalFlags(m_node->LocalFlags & ~ImGuiDockNodeFlags_NoTabBar);
            }
        }
    }

    // --- DockBuilder ---
    DockBuilder::DockBuilder(ImGuiID dockspaceId) : m_dockspaceId(dockspaceId) {}

    void DockNode::setHiddenTabBar(bool hiddenTabBar)
    {
        if (m_node)
        {
            if (hiddenTabBar)
            {
                m_node->SetLocalFlags(m_node->LocalFlags |
                                      ImGuiDockNodeFlags_HiddenTabBar);
            }
            else
            {
                m_node->SetLocalFlags(m_node->LocalFlags &
                                      ~ImGuiDockNodeFlags_HiddenTabBar);
            }
        }
    }

    ImGuiID DockBuilder::splitNode(ImGuiID &nodeIdToSplit, DockDirection dir,
                                   float ratio)
    {
        ImGuiID newNodeId;
        ImGui::DockBuilderSplitNode(nodeIdToSplit, static_cast<ImGuiDir>(dir), ratio,
                                    &newNodeId, &nodeIdToSplit);
        return newNodeId;
    }

    DockNode DockBuilder::getNode(ImGuiID nodeId)
    {
        return DockNode(ImGui::DockBuilderGetNode(nodeId));
    }

    void DockBuilder::dockWindow(const char *windowName, ImGuiID nodeId)
    {
        ImGui::DockBuilderDockWindow(windowName, nodeId);
    }

    void DockBuilder::dockWindow(const std::string &windowName, ImGuiID nodeId)
    {
        ImGui::DockBuilderDockWindow(windowName.c_str(), nodeId);
    }

} // namespace mui