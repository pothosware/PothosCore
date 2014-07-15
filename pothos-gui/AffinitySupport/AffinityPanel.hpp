// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "PothosGui.hpp"
#include "AffinitySupport/AffinityZoneEditor.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QToolTip>
#include <QTabWidget>

class AffinityPanel : public QWidget
{
    Q_OBJECT
public:
    AffinityPanel(QWidget *parent);

private slots:
    void handleCreateZone(void)
    {
        auto zoneName = _zoneEntry->text();
        _zoneEntry->setText("");
        if (zoneName.isEmpty()) return;
        //TODO is in existing list?
        this->handleErrorMessage(zoneName);
        _editorsTabs->addTab(new AffinityZoneEditor(zoneName, this), zoneName);
        this->saveAffinityZoneEditorsState();
    }

private:

    void ensureDefault(void)
    {
        //FIXME not this way
        _editorsTabs->addTab(new AffinityZoneEditor("default", this), "default");
    }

    void initAffinityZoneEditors(void)
    {
        auto names = getSettings().value("AffinityZones/zoneNames").toStringList();
        for (const auto &name : names)
        {
            _editorsTabs->addTab(new AffinityZoneEditor(name, this), name);
        }
        if (names.isEmpty()) this->ensureDefault();
    }

    void saveAffinityZoneEditorsState(void)
    {
        //TODO save it
        this->ensureDefault();
    }

    void handleAffinityZoneEditor(QObject *)
    {
        //TODO maybe something other than destroyed signal to record this
        //objects are always destroyed
        this->saveAffinityZoneEditorsState();
    }

    void handleErrorMessage(const QString &errMsg)
    {
        QToolTip::showText(_zoneEntry->mapToGlobal(QPoint()), "<font color=\"red\">"+errMsg+"</font>");
    }

    QLineEdit *_zoneEntry;
    QPushButton *_createButton;
    QTabWidget *_editorsTabs;
};
