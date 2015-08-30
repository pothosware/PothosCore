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
class QToolButton;
class QRadioButton;
class QButtonGroup;
class PropertyEditWidget;

struct GraphVariableFormData
{
    PropertyEditWidget *editWidget;
    QRadioButton *radioButton;
    QLabel *formLabel;
    QWidget *formWidget;
};

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
    void handleCreateVariable(void);
    void updateAllVariableForms(void);
    void handleVariableRemoval(void);
    void handleVariableMoveUp(void);
    void handleVariableMoveDown(void);
    void handleVariableRemoval(const QString &);
    void handleVariableMoveUp(const QString &);
    void handleVariableMoveDown(const QString &);

private:
    QPointer<GraphEditor> _graphEditor;
    QFormLayout *_formLayout;
    QLineEdit *_varNameEntry;

    //stashed state of global variables
    QStringList _originalVariableNames;
    std::map<QString, QString> _varNameToOriginal;

    //global variables edit widget
    QFormLayout *_varsFormLayout;
    QToolButton *_varsAddButton;
    QToolButton *_varsRemoveButton;
    QToolButton *_varsMoveUpButton;
    QToolButton *_varsMoveDownButton;
    QButtonGroup *_varsSelectionGroup;
    void createVariableEditWidget(const QString &name);
    QStringList getSelectedVariables(void) const;
    std::map<QString, GraphVariableFormData> _varToFormData;

    //get a descriptive list of changes
    QStringList getChangeDescList(void) const;
};
