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
    _idEdit(nullptr),
    _idLabel(nullptr),
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

    //id
    {
        _originalId = _breaker->getId();
        _idEdit = new QLineEdit(this);
        _idLabel = new QLabel(this);
        _formLayout->addRow(_idLabel, _idEdit);
        connect(_idEdit, SIGNAL(textEdited(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        connect(_idEdit, SIGNAL(returnPressed(void)), this, SLOT(handleCommit(void)));
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
    //no changes? do a cancel instead
    auto changed =
        (_breaker->getId() != _originalId) or
        (_breaker->getNodeName() != _originalNodeName);
    if (not changed) return this->handleCancel();

    //format a description
    std::vector<QString> descs;
    if (_breaker->getId() != _originalId) descs.push_back(tr("Breaker ID: %1->%2").arg(_originalId).arg(_breaker->getId()));
    if (_breaker->getNodeName() != _originalNodeName) descs.push_back(tr("Breaker Name: %1->%2").arg(_originalNodeName).arg(_breaker->getNodeName()));
    const auto desc = (descs.size() == 1)? descs.at(0) : tr("Breaker %1 modifications").arg(_breaker->getId());

    //emit the new state
    emit this->stateChanged(GraphState("document-properties", desc));

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

void BreakerPropertiesPanel::handleEditWidgetChanged(const QString &)
{
    _breaker->setId(_idEdit->text());
    _breaker->setNodeName(_nodeNameEdit->text());
    this->update();
}

void BreakerPropertiesPanel::update(void)
{
    //id
    auto idChange = _breaker->getId() != _originalId;
    _idEdit->setText(_breaker->getId());
    _idLabel->setText(QString("<b>%1%2</b>")
        .arg(tr("ID"))
        .arg(idChange?"*":""));

    //node name
    auto nameChange = _breaker->getNodeName() != _originalNodeName;
    _nodeNameEdit->setText(_breaker->getNodeName());
    _nodeNameLabel->setText(QString("<b>%1%2</b>")
        .arg(tr("NodeName"))
        .arg(nameChange?"*":""));
}
