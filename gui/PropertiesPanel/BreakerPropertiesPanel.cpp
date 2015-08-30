// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BreakerPropertiesPanel.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "GraphEditor/Constants.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <cassert>

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

    //info text
    {
        QString info;
        info += QString("<h2>%1 %2</h2>")
            .arg(tr("Breakers on node"))
            .arg(_breaker->getNodeName().toHtmlEscaped());
        for (auto obj : _breaker->draw()->getGraphEditor()->getGraphObjects(GRAPH_BREAKER))
        {
            auto breaker = dynamic_cast<GraphBreaker *>(obj);
            assert(breaker != nullptr);
            if (breaker->getNodeName() != _breaker->getNodeName()) continue;
            info += tr("<h3>%1 %2</h3>")
                .arg(breaker->isInput()?tr("Input Breaker"):tr("Output Breaker"))
                .arg(breaker->getId().toHtmlEscaped());
            info += "<ul>";
            for (auto subObj : _breaker->draw()->getGraphEditor()->getGraphObjects(GRAPH_CONNECTION))
            {
                auto conn = dynamic_cast<GraphConnection *>(subObj);
                assert(conn != nullptr);
                const auto &epOther = breaker->isInput()? conn->getOutputEndpoint() : conn->getInputEndpoint();
                const auto &epSelf = breaker->isInput()? conn->getInputEndpoint() : conn->getOutputEndpoint();
                if (epSelf.getObj() != breaker) continue;
                info += QString("<li>%1[%2]</li>")
                    .arg(epOther.getObj()->getId().toHtmlEscaped())
                    .arg(epOther.getKey().id.toHtmlEscaped());
            }
            info += "</ul>";
        }
        auto text = new QLabel(info, this);
        text->setStyleSheet("QLabel{background:white;margin:1px;}");
        text->setWordWrap(true);
        _formLayout->addRow(text);
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
    if (_breaker->getId() != _originalId) descs.push_back(tr("Breaker ID: %1->%2")
        .arg(_originalId.toHtmlEscaped())
        .arg(_breaker->getId().toHtmlEscaped()));
    if (_breaker->getNodeName() != _originalNodeName) descs.push_back(tr("Breaker Name: %1->%2")
        .arg(_originalNodeName.toHtmlEscaped())
        .arg(_breaker->getNodeName().toHtmlEscaped()));
    const auto desc = (descs.size() == 1)? descs.at(0) : tr("Breaker %1 modifications")
        .arg(_breaker->getId().toHtmlEscaped());

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
        .arg(tr("Node Name"))
        .arg(nameChange?"*":""));
}
