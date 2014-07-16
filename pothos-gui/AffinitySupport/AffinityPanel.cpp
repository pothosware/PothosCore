// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "AffinitySupport/AffinityPanel.hpp"
#include <QToolTip>
#include <QVBoxLayout>
#include <QHBoxLayout>

AffinityPanel::AffinityPanel(QWidget *parent):
    QWidget(parent),
    _zoneEntry(new QLineEdit(this)),
    _createButton(new QPushButton(makeIconFromTheme("list-add"), tr("Create zone"), this)),
    _editorsTabs(new QTabWidget(this))
{
    //layout setup
    auto mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

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
        connect(_editorsTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabCloseRequested(int)));
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

void AffinityPanel::handleTabCloseRequested(const int index)
{
    _editorsTabs->removeTab(index);
    this->ensureDefault();
    this->saveAffinityZoneEditorsState();
}

void AffinityPanel::handleCreateZone(void)
{
    auto zoneName = _zoneEntry->text();
    _zoneEntry->setText("");
    if (zoneName.isEmpty()) return;
    for (int i = 0; i < _editorsTabs->count(); i++)
    {
        if (_editorsTabs->tabText(i) == zoneName)
        {
            this->handleErrorMessage(tr("%1 already exists!").arg(zoneName));
            return;
        }
    }
    auto editor = this->createZoneFromName(zoneName);
    _editorsTabs->setCurrentWidget(editor);
    this->saveAffinityZoneEditorsState();
}

QWidget *AffinityPanel::createZoneFromName(const QString &zoneName)
{
    auto editor = new AffinityZoneEditor(this);
    _editorsTabs->addTab(editor, zoneName);
    return editor;
}

void AffinityPanel::ensureDefault(void)
{
    if (_editorsTabs->count() == 0) this->createZoneFromName("default");
}

void AffinityPanel::initAffinityZoneEditors(void)
{
    auto names = getSettings().value("AffinityZones/zoneNames").toStringList();
    for (const auto &name : names) this->createZoneFromName(name);
    this->ensureDefault();
}

void AffinityPanel::saveAffinityZoneEditorsState(void)
{
    QStringList names;
    for (int i = 0; i < _editorsTabs->count(); i++) names.push_back(_editorsTabs->tabText(i));
    getSettings().setValue("AffinityZones/zoneNames", names);
}

void AffinityPanel::handleErrorMessage(const QString &errMsg)
{
    QToolTip::showText(_zoneEntry->mapToGlobal(QPoint()), "<font color=\"red\">"+errMsg+"</font>");
}

QWidget *makeAffinityPanel(QWidget *parent)
{
    return new AffinityPanel(parent);
}
