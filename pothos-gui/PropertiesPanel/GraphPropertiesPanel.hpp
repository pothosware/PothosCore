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

struct GraphConstantFormData
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
    void handleCreateConstant(void);
    void updateAllConstantForms(void);
    void handleConstRemoval(void);
    void handleConstMoveUp(void);
    void handleConstMoveDown(void);
    void handleConstRemoval(const QString &);
    void handleConstMoveUp(const QString &);
    void handleConstMoveDown(const QString &);

private:
    QPointer<GraphEditor> _graphEditor;
    QFormLayout *_formLayout;
    QLineEdit *_constantNameEntry;

    //stashed state of constants
    QStringList _originalConstNames;
    std::map<QString, QString> _constNameToOriginal;

    //constants edit widget
    QFormLayout *_constantsFormLayout;
    QToolButton *_constantsAddButton;
    QToolButton *_constantsRemoveButton;
    QToolButton *_constantsMoveUpButton;
    QToolButton *_constantsMoveDownButton;
    QButtonGroup *_constantsSelectionGroup;

    //constants entry
    void createConstantEditWidget(const QString &name);
    QStringList getSelectedConstants(void) const;
    std::map<QString, GraphConstantFormData> _constantToFormData;

    //get a descriptive list of changes
    QStringList constValuesChanged(void) const;
};
