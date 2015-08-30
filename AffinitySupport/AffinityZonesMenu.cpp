// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AffinitySupport/AffinityZonesMenu.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include <QSignalMapper>

AffinityZonesMenu::AffinityZonesMenu(AffinityZonesDock *dock, QWidget *parent):
    QMenu(tr("Graph blocks affinity..."), parent),
    _clickMapper(new QSignalMapper(this)),
    _dock(dock)
{
    connect(_clickMapper, SIGNAL(mapped(const QString &)), this, SLOT(handleMapperClicked(const QString &)));
    connect(_dock, SIGNAL(zonesChanged(void)), this, SLOT(handleZonesChanged(void)));
    this->handleZonesChanged(); //init
}

void AffinityZonesMenu::handleZonesChanged(void)
{
    this->clear();

    //clear affinity setting
    auto clearAction = this->addAction(tr("Clear affinity"));
    connect(clearAction, SIGNAL(triggered(void)), _clickMapper, SLOT(map(void)));
    _clickMapper->setMapping(clearAction, "");

    //gui affinity setting
    auto guiAction = this->addAction(tr("GUI affinity"));
    connect(guiAction, SIGNAL(triggered(void)), _clickMapper, SLOT(map(void)));
    _clickMapper->setMapping(guiAction, "gui");

    if (_dock) for (const auto &name : _dock->zones())
    {
        auto action = this->addAction(tr("Apply %1").arg(name));
        connect(action, SIGNAL(triggered(void)), _clickMapper, SLOT(map(void)));
        _clickMapper->setMapping(action, name);
    }
}

void AffinityZonesMenu::handleMapperClicked(const QString &name)
{
    emit this->zoneClicked(name);
}
