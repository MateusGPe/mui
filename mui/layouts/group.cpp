#include "group.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    Group::Group(const std::string &title) : title(title) { App::assertMainThread(); }

    void Group::renderControl()
    {
        if (!visible)
            return;
        ScopedID id(this);

        bool was_open = m_open;

        if (ImGui::CollapsingHeader(title.c_str(), &m_open, m_flags))
        {
            if (margined)
                ImGui::Indent();
            if (child)
                child->render();
            if (margined)
                ImGui::Unindent();
        }

        if (m_open != was_open)
        {
            onToggledSignal(m_open);
        }
    }

    GroupPtr Group::setChild(IControlPtr c)
    {
        child = c;
        return self();
    }
    GroupPtr Group::setMargined(bool m)
    {
        margined = m;
        return self();
    }
    std::string Group::getTitle() const { return title; }
    GroupPtr Group::setTitle(const std::string &t)
    {
        title = t;
        return self();
    }
    bool Group::getMargined() const { return margined; }

    bool Group::isOpen() const { return m_open; }
    GroupPtr Group::setOpen(bool open)
    {
        m_open = open;
        return self();
    }
    GroupPtr Group::onToggled(std::function<void(bool)> cb)
    {
        if (cb)
            m_connections.push_back(onToggledSignal.connect(std::move(cb)));
        return self();
    }
    GroupPtr Group::bindTitle(std::shared_ptr<Observable<std::string>> observable)
    {
        setTitle(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const std::string &val)
                                                                   { mui::App::queueMain([this, val]()
                                                                                         { this->setTitle(val); }); }));
        return self();
    }
    GroupPtr Group::bindOpen(std::shared_ptr<Observable<bool>> observable)
    {
        setOpen(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const bool &val)
                                                                   { mui::App::queueMain([this, val]()
                                                                                         { this->setOpen(val); }); }));
        m_connections.push_back(onToggledSignal.connect([observable](bool val)
                                                        { observable->set(val); }));
        return self();
    }

    GroupPtr Group::defaultOpen(bool open)
    {
        if (open)
            m_flags |= ImGuiTreeNodeFlags_DefaultOpen;
        else
            m_flags &= ~ImGuiTreeNodeFlags_DefaultOpen;
        return self();
    }

    GroupPtr Group::setAllowOverlap(bool allow)
    {
        if (allow)
            m_flags |= ImGuiTreeNodeFlags_AllowOverlap;
        else
            m_flags &= ~ImGuiTreeNodeFlags_AllowOverlap;
        return self();
    }

    GroupPtr Group::setOpenOnDoubleClick(bool open)
    {
        if (open)
            m_flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick;
        else
            m_flags &= ~ImGuiTreeNodeFlags_OpenOnDoubleClick;
        return self();
    }

    GroupPtr Group::setOpenOnArrow(bool open)
    {
        if (open)
            m_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
        else
            m_flags &= ~ImGuiTreeNodeFlags_OpenOnArrow;
        return self();
    }

    GroupPtr Group::setBullet(bool show)
    {
        if (show)
            m_flags |= ImGuiTreeNodeFlags_Bullet;
        else
            m_flags &= ~ImGuiTreeNodeFlags_Bullet;
        return self();
    }

    GroupPtr Group::setFramePadding(bool use)
    {
        if (use)
            m_flags |= ImGuiTreeNodeFlags_FramePadding;
        else
            m_flags &= ~ImGuiTreeNodeFlags_FramePadding;
        return self();
    }

    GroupPtr Group::setSpanAvailWidth(bool span)
    {
        if (span)
            m_flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        else
            m_flags &= ~ImGuiTreeNodeFlags_SpanAvailWidth;
        return self();
    }

    GroupPtr Group::setSpanFullWidth(bool span)
    {
        if (span)
            m_flags |= ImGuiTreeNodeFlags_SpanFullWidth;
        else
            m_flags &= ~ImGuiTreeNodeFlags_SpanFullWidth;
        return self();
    }

    GroupPtr Group::setNavLeftJumpsToParent(bool allow)
    {
        if (allow)
            m_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent;
        else
            m_flags &= ~ImGuiTreeNodeFlags_NavLeftJumpsToParent;
        return self();
    }
    GroupPtr Group::setFramed(bool b)
    {
        if (b)
            m_flags |= ImGuiTreeNodeFlags_Framed;
        else
            m_flags &= ~ImGuiTreeNodeFlags_Framed;
        return self();
    }
    GroupPtr Group::setSelected(bool b)
    {
        if (b)
            m_flags |= ImGuiTreeNodeFlags_Selected;
        else
            m_flags &= ~ImGuiTreeNodeFlags_Selected;
        return self();
    }
    GroupPtr Group::setNoTreePushOnOpen(bool b)
    {
        if (b)
            m_flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
        else
            m_flags &= ~ImGuiTreeNodeFlags_NoTreePushOnOpen;
        return self();
    }
    GroupPtr Group::setNoAutoOpenOnLog(bool b)
    {
        if (b)
            m_flags |= ImGuiTreeNodeFlags_NoAutoOpenOnLog;
        else
            m_flags &= ~ImGuiTreeNodeFlags_NoAutoOpenOnLog;
        return self();
    }
    GroupPtr Group::setLeaf(bool b)
    {
        if (b)
            m_flags |= ImGuiTreeNodeFlags_Leaf;
        else
            m_flags &= ~ImGuiTreeNodeFlags_Leaf;
        return self();
    }
    GroupPtr Group::setSpanAllColumns(bool b)
    {
        if (b)
            m_flags |= ImGuiTreeNodeFlags_SpanAllColumns;
        else
            m_flags &= ~ImGuiTreeNodeFlags_SpanAllColumns;
        return self();
    }
} // namespace mui
