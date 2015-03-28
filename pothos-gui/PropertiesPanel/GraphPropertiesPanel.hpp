// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphEditor/GraphState.hpp"
#include <QWidget>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <string>
#include <map>

class GraphEditor;
class QFormLayout;
class QLineEdit;
class QLabel;
class QPushButton;
class QSignalMapper;
class PropertyEditWidget;

class GraphPropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    GraphPropertiesPanel(GraphEditor *editor, QWidget *parent);

signals:
    void stateChanged(const GraphState &);

public slots:
    void handleCancel(void);
    void handleCommit(void);

private slots:
    void handleCreateConstant(void);
    void updateAllConstantForms(void);
    void handleConstRemoval(const QString &);

private:
    QPointer<GraphEditor> _graphEditor;
    QFormLayout *_formLayout;
    QLineEdit *_constantNameEntry;

    //stashed state of constants
    QStringList _originalConstNames;
    std::map<QString, QString> _constNameToOriginal;

    //constants edit widget
    std::map<QString, PropertyEditWidget *> _constNameToEditWidget;
    std::map<QString, QObjectList> _constNameToObjects;
    QFormLayout *_constNameFormLayout;
    QSignalMapper *_constRemovalMapper;
    void createConstantEditWidget(const QString &name);

    //get a descriptive list of changes
    QStringList constValuesChanged(void) const;
};
