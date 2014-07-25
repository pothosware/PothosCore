// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <QStringList>
#include <QColor>

class QLineEdit;
class QPushButton;
class QTabWidget;
class AffinityZoneEditor;

class AffinityPanel : public QWidget
{
    Q_OBJECT
public:
    AffinityPanel(QWidget *parent);

    //! query the list of available zones
    QStringList zones(void) const;

    //! Get the color for a particular zone
    QColor zoneToColor(const QString &zone);

    //! Get the config for a particular zone
    Poco::JSON::Object::Ptr zoneToConfig(const QString &zone);

signals:

    //! emitted when zones are created, destroyed, changed
    void zonesChanged(void);

private slots:
    void handleCreateZone(void);
    void handleTabCloseRequested(int);
    void handleZoneEditorChanged(void)
    {
        this->saveAffinityZoneEditorsState();
    }
    void handleTabSelectionChanged(int)
    {
        this->saveAffinityZoneEditorsState();
    }

private:
    AffinityZoneEditor *createZoneFromName(const QString &name);

    void ensureDefault(void);

    void initAffinityZoneEditors(void);

    void saveAffinityZoneEditorsState(void);

    void handleErrorMessage(const QString &errMsg);

    QLineEdit *_zoneEntry;
    QPushButton *_createButton;
    QTabWidget *_editorsTabs;
};
