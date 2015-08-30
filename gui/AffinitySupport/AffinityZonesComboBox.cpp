// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AffinitySupport/AffinityZonesComboBox.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include "ColorUtils/ColorUtils.hpp"

AffinityZonesComboBox::AffinityZonesComboBox(AffinityZonesDock *dock, QWidget *parent):
    QComboBox(parent),
    _dock(dock)
{
    this->handleZonesChanged(); //initial update
    connect(_dock, SIGNAL(zonesChanged(void)), this, SLOT(handleZonesChanged(void)));
}

void AffinityZonesComboBox::handleZonesChanged(void)
{
    auto oldSelection = this->itemData(this->currentIndex()).toString();
    this->blockSignals(true);

    //reload the options
    this->clear();
    this->addItem(tr("Select affinity zone..."), "");
    this->addItem(tr("GUI"), "gui");
    this->setCurrentIndex(0);
    for (const auto &zone : _dock->zones())
    {
        this->addItem(zone, zone);
        this->setItemData(this->count()-1, colorToWidgetIcon(_dock->zoneToColor(zone)), Qt::DecorationRole);
        if (zone == oldSelection) this->setCurrentIndex(this->count()-1);
    }

    this->blockSignals(false);
}
