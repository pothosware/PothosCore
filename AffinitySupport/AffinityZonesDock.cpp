// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "AffinitySupport/AffinityZonesDock.hpp"
#include "AffinitySupport/AffinityZonesMenu.hpp"
#include "AffinitySupport/AffinityZonesComboBox.hpp"
#include "AffinitySupport/AffinityZoneEditor.hpp"
#include "ColorUtils/ColorUtils.hpp"
#include <QToolTip>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QSignalMapper>
#include <Poco/JSON/Parser.h>
#include <Poco/Logger.h>
#include <cassert>
#include <sstream>

AffinityZonesDock::AffinityZonesDock(QWidget *parent):
    QDockWidget(parent),
    _mapper(new QSignalMapper(this)),
    _zoneEntry(new QLineEdit(this)),
    _createButton(new QPushButton(makeIconFromTheme("list-add"), tr("Create zone"), this)),
    _editorsTabs(new QTabWidget(this))
{
    this->setObjectName("AffinityZonesDock");
    this->setWindowTitle(tr("Affinity Zones"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    this->setWidget(new QWidget(this));

    //layout setup
    auto mainLayout = new QVBoxLayout(this->widget());
    this->widget()->setLayout(mainLayout);

    //editors area
    {
        mainLayout->addWidget(_editorsTabs);
        _editorsTabs->setTabsClosable(true);
        _editorsTabs->setMovable(true);
        _editorsTabs->setUsesScrollButtons(true);
        _editorsTabs->setTabPosition(QTabWidget::North);
        _editorsTabs->setStyleSheet(
            QString("QTabBar::close-button {image: url(%1);}").arg(makeIconPath("standardbutton-closetab-16.png"))+
            QString("QTabBar::close-button:hover {image: url(%1);}").arg(makeIconPath("standardbutton-closetab-hover-16.png"))+
            QString("QTabBar::close-button:pressed {image: url(%1);}").arg(makeIconPath("standardbutton-closetab-down-16.png")));
        connect(_mapper, SIGNAL(mapped(const QString &)), this, SIGNAL(zoneChanged(const QString &)));
    }

    //zone creation area
    {
        auto hbox = new QHBoxLayout();
        mainLayout->addLayout(hbox);
        hbox->addWidget(_zoneEntry);
        hbox->addWidget(_createButton);
        _zoneEntry->setPlaceholderText(tr("Enter a new zone name..."));
        _createButton->setToolTip(tr("Create a new affinity zone editor panel."));
        connect(_zoneEntry, SIGNAL(returnPressed(void)), this, SLOT(handleCreateZone(void)));
        connect(_createButton, SIGNAL(pressed(void)), this, SLOT(handleCreateZone(void)));
    }

    this->initAffinityZoneEditors();
}

QMenu *AffinityZonesDock::makeMenu(QWidget *parent)
{
    return new AffinityZonesMenu(this, parent);
}

QComboBox *AffinityZonesDock::makeComboBox(QWidget *parent)
{
    return new AffinityZonesComboBox(this, parent);
}

QStringList AffinityZonesDock::zones(void) const
{
    QStringList zones;
    for (int i = 0; i < _editorsTabs->count(); i++) zones.push_back(_editorsTabs->tabText(i));
    return zones;
}

QColor AffinityZonesDock::zoneToColor(const QString &zone)
{
    for (int i = 0; i < _editorsTabs->count(); i++)
    {
        if (zone == _editorsTabs->tabText(i))
        {
            return dynamic_cast<AffinityZoneEditor *>(_editorsTabs->widget(i))->color();
        }
    }
    return QColor();
}

Poco::JSON::Object::Ptr AffinityZonesDock::zoneToConfig(const QString &zone)
{
    for (int i = 0; i < _editorsTabs->count(); i++)
    {
        if (zone == _editorsTabs->tabText(i))
        {
            return dynamic_cast<AffinityZoneEditor *>(_editorsTabs->widget(i))->getCurrentConfig();
        }
    }
    return Poco::JSON::Object::Ptr();
}

void AffinityZonesDock::handleTabCloseRequested(const int index)
{
    _editorsTabs->removeTab(index);
    this->ensureDefault();
    this->saveAffinityZoneEditorsState();
}

void AffinityZonesDock::handleCreateZone(void)
{
    auto zoneName = _zoneEntry->text();
    _zoneEntry->setText("");
    if (zoneName.isEmpty()) return;
    for (const auto &name : this->zones())
    {
        if (name == zoneName)
        {
            this->handleErrorMessage(tr("%1 already exists!").arg(zoneName));
            return;
        }
    }
    auto editor = this->createZoneFromName(zoneName);
    _editorsTabs->setCurrentWidget(editor);
    this->saveAffinityZoneEditorsState();
}

AffinityZoneEditor *AffinityZonesDock::createZoneFromName(const QString &zoneName)
{
    auto editor = new AffinityZoneEditor(this);
    _editorsTabs->addTab(editor, zoneName);
    if (zoneName == getSettings().value("AffinityZones/currentZone").toString()) _editorsTabs->setCurrentWidget(editor);

    //restore the settings from save -- even if this is a new panel with the same name as a previous one
    auto json = getSettings().value("AffinityZones/zones/"+zoneName).toString();
    if (not json.isEmpty()) try
    {
        Poco::JSON::Parser p; p.parse(json.toStdString());
        auto dataObj = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
        editor->loadFromConfig(dataObj);
    }
    catch (const Poco::JSON::JSONException &ex)
    {
        poco_error_f2(Poco::Logger::get("PothosGui.AffinityZonesDock"), "Failed to load editor for zone '%s' -- %s", zoneName.toStdString(), ex.displayText());
    }

    //now connect the changed signal after initialization+restore changes
    connect(editor, SIGNAL(settingsChanged(void)), this, SLOT(handleZoneEditorChanged(void)));
    connect(editor, SIGNAL(settingsChanged(void)), _mapper, SLOT(map(void)));
    _mapper->setMapping(editor, zoneName);

    //when to update colors
    connect(editor, SIGNAL(settingsChanged(void)), this, SLOT(updateTabColors(void)));
    this->updateTabColors();

    return editor;
}

void AffinityZonesDock::ensureDefault(void)
{
    if (_editorsTabs->count() == 0) this->createZoneFromName("default");
}

void AffinityZonesDock::initAffinityZoneEditors(void)
{
    auto names = getSettings().value("AffinityZones/zoneNames").toStringList();
    for (const auto &name : names) this->createZoneFromName(name);
    this->ensureDefault();
    connect(_editorsTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabCloseRequested(int)));
    connect(_editorsTabs, SIGNAL(currentChanged(int)), this, SLOT(handleTabSelectionChanged(int)));
}

void AffinityZonesDock::updateTabColors(void)
{
    for (int i = 0; i < _editorsTabs->count(); i++)
    {
        auto editor = dynamic_cast<AffinityZoneEditor *>(_editorsTabs->widget(i));
        _editorsTabs->setTabIcon(i, colorToWidgetIcon(editor->color()));
    }
}

void AffinityZonesDock::handleTabSelectionChanged(const int index)
{
    getSettings().setValue("AffinityZones/currentZone", _editorsTabs->tabText(index));
}

void AffinityZonesDock::saveAffinityZoneEditorsState(void)
{
    getSettings().setValue("AffinityZones/zoneNames", this->zones());

    for (int i = 0; i < _editorsTabs->count(); i++)
    {
        auto editor = dynamic_cast<AffinityZoneEditor *>(_editorsTabs->widget(i));
        assert(editor != nullptr);
        auto dataObj = editor->getCurrentConfig();
        std::stringstream ss; dataObj->stringify(ss);
        getSettings().setValue("AffinityZones/zones/"+_editorsTabs->tabText(i), QString::fromStdString(ss.str()));
    }

    emit this->zonesChanged();
}

void AffinityZonesDock::handleErrorMessage(const QString &errMsg)
{
    QToolTip::showText(_zoneEntry->mapToGlobal(QPoint()), "<font color=\"red\">"+errMsg+"</font>");
}
