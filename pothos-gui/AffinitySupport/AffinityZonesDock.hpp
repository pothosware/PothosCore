// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <QDockWidget>

class AffinityPanel;
class QMenu;

//! Top level dock widget for affinity designer
class AffinityZonesDock : public QDockWidget
{
    Q_OBJECT
public:
    AffinityZonesDock(QWidget *parent);

    //! Make a new affinity selection menu that updates with this panel's configuration
    QMenu *makeMenu(QWidget *parent);

    //! query the list of available zones
    QStringList zones(void) const;

signals:

    //! emitted when zones are created, destroyed, changed
    void zonesChanged(void);

private:
    AffinityPanel *_panel;
};
