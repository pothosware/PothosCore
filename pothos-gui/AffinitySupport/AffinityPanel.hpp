// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "PothosGui.hpp"
#include "AffinitySupport/AffinityZoneEditor.hpp"
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <iostream>

class AffinityPanel : public QWidget
{
    Q_OBJECT
public:
    AffinityPanel(QWidget *parent);

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
