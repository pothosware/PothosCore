// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "GraphPropertiesPanel.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolTip>
#include <iostream>

GraphPropertiesPanel::GraphPropertiesPanel(GraphEditor *editor, QWidget *parent):
    QWidget(parent),
    _graphEditor(editor),
    _formLayout(new QFormLayout(this)),
    _constantNameEntry(nullptr)
{
    //title
    {
        auto label = new QLabel(tr("<h1>%1</h1>")
            .arg(tr("Graph Properties")), this);
        label->setAlignment(Qt::AlignCenter);
        _formLayout->addRow(label);
    }

    //graph info
    {
        auto label = new QLabel(tr("<p>%2</p>")
            .arg(_graphEditor->getCurrentFilePath()), this);
        _formLayout->addRow(label);
    }

    //constants editor
    {
        auto constantsBox = new QGroupBox(tr("Graph Constants"), this);
        auto constantsBoxLayout = new QVBoxLayout(constantsBox);

        auto nameEntryLayout = new QHBoxLayout();
        _constantNameEntry = new QLineEdit(constantsBox);
        _constantNameEntry->setPlaceholderText(tr("Enter a new constant name"));
        auto nameEntryButton = new QPushButton(makeIconFromTheme("list-add"), tr("Create"), constantsBox);
        connect(nameEntryButton, SIGNAL(clicked(void)), this, SLOT(handleCreateConstant(void)));
        connect(_constantNameEntry, SIGNAL(returnPressed(void)), this, SLOT(handleCreateConstant(void)));
        nameEntryLayout->addWidget(_constantNameEntry);
        nameEntryLayout->addWidget(nameEntryButton);
        constantsBoxLayout->addLayout(nameEntryLayout);

        _formLayout->addRow(constantsBox);
    }
}

void GraphPropertiesPanel::handleCancel(void)
{
    
}

void GraphPropertiesPanel::handleCommit(void)
{
    
}

void GraphPropertiesPanel::handleCreateConstant(void)
{
    const auto name = _constantNameEntry->text().trimmed();
    _constantNameEntry->setText(QString());

    //empty name, do nothing
    if (name.isEmpty()) return;

    QString errorMsg;

    //check for a legal variable name
    if (name.count(QRegExp("^[a-zA-Z]\\w*$")) != 1)
    {
        errorMsg = tr("Not a legal variable name: '%1'").arg(name);
    }

    if (not errorMsg.isEmpty()) QToolTip::showText(_constantNameEntry->mapToGlobal(QPoint()), "<font color=\"red\">"+errorMsg+"</font>");
}
