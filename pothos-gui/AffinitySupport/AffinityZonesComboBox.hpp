// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QComboBox>
#include <QPointer>

class AffinityPanel;

/*!
 * A combo box with options that reflect the active affinity zones.
 */
class AffinityZonesComboBox : public QComboBox
{
    Q_OBJECT
public:
    AffinityZonesComboBox(AffinityPanel *affinityPanel, QWidget *parent);

private slots:
    void handleZonesChanged(void);

private:
    QPointer<AffinityPanel> _affinityPanel;
};
