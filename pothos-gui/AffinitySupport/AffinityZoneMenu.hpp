// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QString>
#include <QMenu>
#include <QPointer>

class QSignalMapper;
class AffinityPanel;

/*!
 * A menu with options that reflect the active affinity zones.
 */
class AffinityZoneMenu : public QMenu
{
    Q_OBJECT
public:
    AffinityZoneMenu(AffinityPanel *affinityPanel, QWidget *parent);

signals:
    //! emitted when a submenu of that zone name is clicked
    void zoneClicked(const QString &);

private slots:
    void handleZonesChanged(void);
    void handleMapperClicked(const QString &);

private:
    QSignalMapper *_clickMapper;
    QPointer<AffinityPanel> _affinityPanel;
};
