// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QWidget>
#include <QPointer>
#include <QString>
#include "GraphEditor/GraphState.hpp"

class GraphBreaker;
class QFormLayout;
class QLineEdit;
class QLabel;

class BreakerPropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    BreakerPropertiesPanel(GraphBreaker *breaker, QWidget *parent);

signals:
    void stateChanged(const GraphState &);

public slots:
    void handleCancel(void);
    void handleCommit(void);

private slots:
    void handleEditWidgetChanged(const QString &);

private:
    void update(void);

    QPointer<GraphBreaker> _breaker;
    QFormLayout *_formLayout;
    QLineEdit *_nodeNameEdit;
    QLabel *_nodeNameLabel;
    QString _originalNodeName;
};
