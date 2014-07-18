// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QString>
#include "GraphEditor/GraphState.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <Poco/JSON/Object.h>
#include <map>

class GraphBlock;
class QLabel;
class QLineEdit;
class QTimer;
class QFormLayout;

class BlockPropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    BlockPropertiesPanel(GraphBlock *block, QWidget *parent);

    QString getParamDocString(const Poco::JSON::Object::Ptr &paramDesc);

private slots:

    void handleBlockDestroyed(QObject *);

    void handleEditWidgetChanged(const QString &);

    void handleUpdateTimerExpired(void);

    void handleCancelButton(void);

    void handleCommitButton(void);

signals:
    void stateChanged(const GraphState &);

private:

    std::map<QString, QString> _propIdToOriginal;
    std::map<QString, QLabel *> _propIdToFormLabel;
    std::map<QString, QLabel *> _propIdToErrorLabel;
    std::map<QString, QWidget *> _propIdToEditWidget;

    /*!
     * Update everything in this panel after a block change
     */
    void updateAllForms(void);

    /*!
     * Update all the things that change when a property is modified.
     * Label string formatting, color of the box, tooltip...
     */
    void updatePropForms(const GraphBlockProp &prop);

    bool _ignoreChanges;
    QString _idOriginal;
    QLabel *_idLabel;
    QLineEdit *_idLineEdit;
    QLabel *_blockErrorLabel;
    QTimer *_updateTimer;
    QFormLayout *_formLayout;
    QPointer<GraphBlock> _block;
};
