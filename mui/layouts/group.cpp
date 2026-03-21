#include "group.hpp"
#include "app.hpp"
#include <imgui.h>
#include "../core/scoped.hpp"

namespace mui
{
    Group::Group(const std::string &title) : title(title) { App::assertMainThread(); }

    void Group::renderControl()
    {
        if (!visible) return;
        ScopedID id(this);

        bool was_open = m_open;

        if (ImGui::CollapsingHeader(title.c_str(), &m_open))
        {
            if (margined) ImGui::Indent();
            if (child) child->render();
            if (margined) ImGui::Unindent();
        }

        if (m_open != was_open)
        {
            onToggledSignal(m_open);
        }
    }

    GroupPtr Group::setChild(IControlPtr c) { child = c; return self(); }
    GroupPtr Group::setMargined(bool m) { margined = m; return self(); }
    std::string Group::getTitle() const { return title; }
    GroupPtr Group::setTitle(const std::string &t) { title = t; return self(); }
    bool Group::getMargined() const { return margined; }

    bool Group::isOpen() const { return m_open; }
    GroupPtr Group::setOpen(bool open) { m_open = open; return self(); }
    GroupPtr Group::onToggled(std::function<void(bool)> cb)
    {
        if (cb) m_connections.push_back(onToggledSignal.connect(std::move(cb)));
        return self();
    }
    GroupPtr Group::bindTitle(std::shared_ptr<Observable<std::string>> observable)
    {
        setTitle(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const std::string &val) { mui::App::queueMain([this, val]() { this->setTitle(val); }); }));
        return self();
    }
    GroupPtr Group::bindOpen(std::shared_ptr<Observable<bool>> observable)
    {
        setOpen(observable->get());
        m_connections.push_back(observable->onValueChanged.connect([this](const bool &val) { mui::App::queueMain([this, val]() { this->setOpen(val); }); }));
        m_connections.push_back(onToggledSignal.connect([observable](bool val) { observable->set(val); }));
        return self();
    }
} // namespace mui
