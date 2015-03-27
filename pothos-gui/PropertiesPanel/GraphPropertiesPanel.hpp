// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphEditor/GraphState.hpp"
#include <QWidget>
#include <QPointer>
#include <QString>
#include <QStringList>
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

private:
    QPointer<GraphEditor> _graphEditor;
    QFormLayout *_formLayout;
    QLineEdit *_constantNameEntry;

    //constants expression entry
    QStringList _orderedConstNames;
    std::map<QString, QString> _constNameToOriginal;
    std::map<QString, QLabel *> _constNameToFormLabel;
    std::map<QString, QLabel *> _constNameToErrorLabel;
    std::map<QString, QWidget *> _constNameToEditWidget;
    QFormLayout *_constNameFormLayout;

    bool constValuesChanged(void) const;
    void updateAllConstantForms(void);
    void updateConstantForm(const QString &name);
};
