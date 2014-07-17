// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AffinitySupport/AffinityZoneMenu.hpp"
#include "AffinitySupport/AffinityPanel.hpp"
#include <QSignalMapper>

AffinityZoneMenu::AffinityZoneMenu(AffinityPanel *affinityPanel, QWidget *parent):
    QMenu(tr("Set graph objects affinity..."), parent),
    _clickMapper(new QSignalMapper(this)),
    _affinityPanel(affinityPanel)
{
    connect(_clickMapper, SIGNAL(mapped(const QString &)), this, SLOT(handleMapperClicked(const QString &)));
    connect(_affinityPanel, SIGNAL(zonesChanged(void)), this, SLOT(handleZonesChanged(void)));
    this->handleZonesChanged(); //init
}

void AffinityZoneMenu::handleZonesChanged(void)
{
    this->clear();
    if (_affinityPanel) for (const auto &name : _affinityPanel->zones())
    {
        auto action = this->addAction(name);
        connect(action, SIGNAL(triggered(void)), _clickMapper, SLOT(map(void)));
        _clickMapper->setMapping(action, name);
    }
}

void AffinityZoneMenu::handleMapperClicked(const QString &name)
{
    emit this->zoneClicked(name);
}
