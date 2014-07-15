// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGui.hpp"
#include "AffinitySupport/AffinityPanel.hpp"

AffinityPanel::AffinityPanel(QWidget *parent):
    QWidget(parent),
    _zoneEntry(new QLineEdit(this)),
    _createButton(new QPushButton(makeIconFromTheme("list-add"), tr("Create"), this)),
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
    }

    //zone creation area
    {
        auto formLayout = new QFormLayout();
        mainLayout->addLayout(formLayout);
        auto hbox = new QHBoxLayout();
        hbox->addWidget(_zoneEntry);
        hbox->addWidget(_createButton);
        _zoneEntry->setPlaceholderText(tr("The name of a new affinity zone"));
        _createButton->setToolTip(tr("Create a new affinity zone"));
        formLayout->addRow(tr("Zone name: "), hbox);
        connect(_zoneEntry, SIGNAL(returnPressed(void)), this, SLOT(handleCreateZone(void)));
        connect(_createButton, SIGNAL(pressed(void)), this, SLOT(handleCreateZone(void)));
    }

    this->initAffinityZoneEditors();
}

QWidget *makeAffinityPanel(QWidget *parent)
{
    return new AffinityPanel(parent);
}
