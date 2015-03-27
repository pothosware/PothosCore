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

private:
    QPointer<GraphEditor> _graphEditor;
    QFormLayout *_formLayout;
    QLineEdit *_constantNameEntry;

    //stashed state of constants
    QStringList _originalConstNames;
    std::map<QString, QString> _constNameToOriginal;

    //current state of constants
    std::map<QString, QLabel *> _constNameToFormLabel;
    std::map<QString, QLabel *> _constNameToErrorLabel;
    std::map<QString, QWidget *> _constNameToEditWidget;
    QFormLayout *_constNameFormLayout;

    //get a descriptive list of changes
    QStringList constValuesChanged(void) const;
    void updateConstantForm(const QString &name);

    //get edit widget (same used by block properties)
    QWidget *makePropertyEditWidget(const std::string &widgetType="LineEdit");
};
