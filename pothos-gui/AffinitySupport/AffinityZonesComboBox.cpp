// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AffinitySupport/AffinityZonesComboBox.hpp"
#include "AffinitySupport/AffinityPanel.hpp"
#include <QPixmap>

AffinityZonesComboBox::AffinityZonesComboBox(AffinityPanel *affinityPanel, QWidget *parent):
    QComboBox(parent),
    _affinityPanel(affinityPanel)
{
    this->handleZonesChanged(); //initial update
    connect(_affinityPanel, SIGNAL(zonesChanged(void)), this, SLOT(handleZonesChanged(void)));
}

void AffinityZonesComboBox::handleZonesChanged(void)
{
    auto oldSelection = this->itemData(this->currentIndex()).toString();
    this->blockSignals(true);

    //reload the options
    this->clear();
    this->addItem(tr("Select affinity zone..."), "");
    this->setCurrentIndex(0);
    for (const auto &zone : _affinityPanel->zones())
    {
        this->addItem(zone, zone);
        QPixmap pixmap(15, 15);
        pixmap.fill(_affinityPanel->zoneToColor(zone));
        this->setItemData(this->count()-1, pixmap, Qt::DecorationRole);
        if (zone == oldSelection) this->setCurrentIndex(this->count()-1);
    }

    this->blockSignals(false);
}
