// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AffinitySupport/AffinityZonesDock.hpp"
#include "AffinitySupport/AffinityZonesMenu.hpp"
#include "AffinitySupport/AffinityPanel.hpp"

AffinityZonesDock::AffinityZonesDock(QWidget *parent):
    QDockWidget(parent),
    _panel(new AffinityPanel(this))
{
    this->setObjectName("AffinityZonesDock");
    this->setWindowTitle(tr("Affinity Zones"));
    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    this->setWidget(_panel);

    //forward zones changed signal
    connect(_panel, SIGNAL(zonesChanged(void)), this, SIGNAL(zonesChanged(void)));
}

QMenu *AffinityZonesDock::makeMenu(QWidget *parent)
{
    return new AffinityZonesMenu(_panel, parent);
}

QStringList AffinityZonesDock::zones(void) const
{
    return _panel->zones();
}
