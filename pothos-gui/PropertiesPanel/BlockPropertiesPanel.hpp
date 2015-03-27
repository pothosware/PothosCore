// Copyright (c) 2014-2015 Josh Blum
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
class QTabWidget;
class PropertyEditWidget;

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

    //! Handle for widget change events -- immediate re-eval
    void handleWidgetChanged(void) {this->handleChange(true);}

    //! Handle for widget change events -- immediate re-eval
    void handleWidgetChanged(const QString &) {this->handleWidgetChanged();}

    //! Handle for text entry changes -- delayed re-eval
    void handleEntryChanged(void) {this->handleChange(false);}

    //! Handle for text entry changes -- delayed re-eval
    void handleEntryChanged(const QString &) {this->handleEntryChanged();}

    void handleUpdateTimerExpired(void);

    void handleBlockEvalDone(void);

    void handleDocTabChanged(int);

signals:
    void stateChanged(const GraphState &);

private:

    std::map<QString, PropertyEditWidget *> _propIdToEditWidget;

    /*!
     * Actual handler for changes.
     * Can be used for immediate re-eval or time-delay.
     * \param immediate true for immediate re-eval
     */
    void handleChange(const bool immediate);

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
    QTabWidget *_infoTabs;
    QLabel *_blockInfoDesc;
    QLabel *_jsonBlockDesc;
    QLabel *_evalTypesDesc;
    QFormLayout *_formLayout;
    std::map<std::string, QFormLayout *> _paramLayouts;
    QPointer<GraphBlock> _block;
};
