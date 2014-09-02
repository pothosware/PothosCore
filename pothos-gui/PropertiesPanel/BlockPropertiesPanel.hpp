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
class QComboBox;
class QFormLayout;
class BlockPropertyEditWidget;

class BlockPropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    BlockPropertiesPanel(GraphBlock *block, QWidget *parent);

    QString getParamDocString(const QString &propKey);

public slots:
    void handleCancel(void);
    void handleCommit(void);

private slots:

    void handleBlockDestroyed(QObject *);

    void handleEditWidgetChanged(const QString &)
    {
        this->handleEditWidgetChanged();
    }

    void handleEditWidgetChanged(void);

    void handleUpdateTimerExpired(void);

    void handleDocTabChanged(int);

signals:
    void stateChanged(const GraphState &);

private:

    std::map<QString, QString> _propIdToOriginal;
    std::map<QString, QLabel *> _propIdToFormLabel;
    std::map<QString, QLabel *> _propIdToErrorLabel;
    std::map<QString, BlockPropertyEditWidget *> _propIdToEditWidget;

    /*!
     * Update everything in this panel after a block change
     */
    void updateAllForms(void);

    /*!
     * Update all the things that change when a property is modified.
     * Label string formatting, color of the box, tooltip...
     */
    void updatePropForms(const QString &propKey);

    bool _ignoreChanges;

    QString _idOriginal;
    QLabel *_idLabel;
    QLineEdit *_idLineEdit;

    QString _affinityZoneOriginal;
    QLabel *_affinityZoneLabel;
    QComboBox *_affinityZoneBox;

    QLabel *_blockErrorLabel;
    QTimer *_updateTimer;
    QLabel *_jsonBlockDesc;
    QFormLayout *_formLayout;
    QPointer<GraphBlock> _block;
};
