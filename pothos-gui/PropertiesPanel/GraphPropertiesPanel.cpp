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
    _constantNameEntry(nullptr),
    _constNameFormLayout(nullptr)
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
        _orderedConstNames = _graphEditor->globals().listGlobals();

        auto constantsBox = new QGroupBox(tr("Graph Constants"), this);
        auto constantsLayout = new QVBoxLayout(constantsBox);
        _constNameFormLayout = new QFormLayout();

        auto nameEntryLayout = new QHBoxLayout();
        _constantNameEntry = new QLineEdit(constantsBox);
        _constantNameEntry->setPlaceholderText(tr("Enter a new constant name"));
        auto nameEntryButton = new QPushButton(makeIconFromTheme("list-add"), tr("Create"), constantsBox);
        connect(nameEntryButton, SIGNAL(clicked(void)), this, SLOT(handleCreateConstant(void)));
        connect(_constantNameEntry, SIGNAL(returnPressed(void)), this, SLOT(handleCreateConstant(void)));
        nameEntryLayout->addWidget(_constantNameEntry);
        nameEntryLayout->addWidget(nameEntryButton);

        constantsLayout->addLayout(_constNameFormLayout);
        constantsLayout->addLayout(nameEntryLayout);

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
        errorMsg = tr("'%1' is not a legal variable name").arg(name);
    }

    //check if the variable exists
    if (std::find(_orderedConstNames.begin(), _orderedConstNames.end(), name) != _orderedConstNames.end())
    {
        errorMsg = tr("Constant '%1' already exists").arg(name);
    }

    //report error and exit
    if (not errorMsg.isEmpty())
    {
        QToolTip::showText(_constantNameEntry->mapToGlobal(QPoint()), "<font color=\"red\">"+errorMsg+"</font>");
        return;
    }

    //success, add the form
    _orderedConstNames.push_back(name);
    this->updateConstantForm(name);
}

void GraphPropertiesPanel::updateAllConstantForms(void)
{
    for (const auto &name : _orderedConstNames)
    {
        this->updateConstantForm(name);
    }
}

void GraphPropertiesPanel::updateConstantForm(const QString &name)
{
    //create the widgets if when they do not exist
    if (_constNameToEditWidget.count(name) == 0)
    {
        auto formLabel = new QLabel(this);
        auto errorLabel = new QLabel(this);
        auto editWidget = new QLineEdit(this);
        auto editLayout = new QVBoxLayout();
        editLayout->addWidget(editWidget);
        editLayout->addWidget(errorLabel);
        _constNameFormLayout->addRow(formLabel, editLayout);

        //TODO what if its blank... _constNameToOriginal[name] = _graphEditor->globals()->getGlobalExpression;
        _constNameToFormLabel[name] = formLabel;
        _constNameToErrorLabel[name] = errorLabel;
        _constNameToEditWidget[name] = editWidget;
    }

    //update the widgets for this constant
    _constNameToFormLabel[name]->setText(name);
    //TODO
}
