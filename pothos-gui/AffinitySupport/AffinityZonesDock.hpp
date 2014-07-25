// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <QDockWidget>
#include <QString>
#include <QColor>

class AffinityPanel;
class QMenu;
class QComboBox;

//! Top level dock widget for affinity designer
class AffinityZonesDock : public QDockWidget
{
    Q_OBJECT
public:
    AffinityZonesDock(QWidget *parent);

    //! Make a new affinity selection menu that updates with this panel's configuration
    QMenu *makeMenu(QWidget *parent);

    //! Make a new affinity combo box that updates with this panel's configuration
    QComboBox *makeComboBox(QWidget *parent);

    //! query the list of available zones
    QStringList zones(void) const;

    //! Get the color for a particular zone
    QColor zoneToColor(const QString &zone);

    //! Get the config for a particular zone
    Poco::JSON::Object::Ptr zoneToConfig(const QString &zone);

signals:

    //! emitted when zones are created, destroyed, changed
    void zonesChanged(void);

    //! list of current zones version
    void zonesChanged(const QStringList &zones);

private slots:
    void handleZonesChanged(void);

private:
    AffinityPanel *_panel;
};
