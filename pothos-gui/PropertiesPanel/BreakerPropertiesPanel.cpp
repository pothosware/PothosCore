// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BreakerPropertiesPanel.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>

BreakerPropertiesPanel::BreakerPropertiesPanel(GraphBreaker *breaker, QWidget *parent):
    QWidget(parent),
    _breaker(breaker),
    _formLayout(nullptr),
    _nodeNameEdit(nullptr),
    _nodeNameLabel(nullptr)
{
    //master layout for this widget
    _formLayout = new QFormLayout(this);

    //title
    {
        auto label = new QLabel(tr("<h1>%1</h1>")
            .arg(_breaker->isInput()?tr("Input Breaker"):tr("Output Breaker")), this);
        label->setAlignment(Qt::AlignCenter);
        _formLayout->addRow(label);
    }

    //node name
    {
        _originalNodeName = _breaker->getNodeName();
        _nodeNameEdit = new QLineEdit(this);
        _nodeNameLabel = new QLabel(this);
        _formLayout->addRow(_nodeNameLabel, _nodeNameEdit);
        connect(_nodeNameEdit, SIGNAL(textEdited(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        connect(_nodeNameEdit, SIGNAL(returnPressed(void)), this, SLOT(handleCommit(void)));
    }

    this->update(); //initialize
}

void BreakerPropertiesPanel::handleCancel(void)
{
    _breaker->setNodeName(_originalNodeName);

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

void BreakerPropertiesPanel::handleCommit(void)
{
    auto changed = _breaker->getNodeName() != _originalNodeName;
    if (not changed) return this->handleCancel();

    const auto desc = tr("Breaker %1->%2")
        .arg(_originalNodeName)
        .arg(_breaker->getNodeName());
    emit this->stateChanged(GraphState("document-properties", desc));

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

void BreakerPropertiesPanel::handleEditWidgetChanged(const QString &name)
{
    _breaker->setNodeName(name);
    this->update();
}

void BreakerPropertiesPanel::update(void)
{
    auto changed = _breaker->getNodeName() != _originalNodeName;
    _nodeNameEdit->setText(_breaker->getNodeName());
    _nodeNameLabel->setText(QString("<b>%1%2</b>")
        .arg(tr("NodeName"))
        .arg(changed?"*":""));
}
