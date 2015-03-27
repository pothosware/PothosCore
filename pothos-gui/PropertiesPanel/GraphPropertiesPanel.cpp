// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "GraphPropertiesPanel.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <Pothos/Plugin.hpp>
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

    //make a backup of constants
    _originalConstNames = _graphEditor->listGlobals();
    for (const auto &name : _originalConstNames)
    {
        _constNameToOriginal[name] = _graphEditor->getGlobalExpression(name);
    }

    //cause the generation of entry forms for existing constants
    this->updateAllConstantForms();
}

void GraphPropertiesPanel::handleCancel(void)
{
    //revert values
    _graphEditor->clearGlobals();
    for (const auto &name : _originalConstNames)
    {
        _graphEditor->setGlobalExpression(name, _constNameToOriginal.at(name));
    }

    //emit re-evaluation
    _graphEditor->commitGlobalsChanges();

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

void GraphPropertiesPanel::handleCommit(void)
{
    //process through changes before inspecting
    this->updateAllConstantForms();

    //look for changes
    const auto propertiesModified = this->constValuesChanged();
    if (propertiesModified.empty()) return this->handleCancel();

    //emit state change
    auto desc = (propertiesModified.size() == 1)? propertiesModified.front() : tr("Modified constants");
    emit this->stateChanged(GraphState("document-properties", desc));

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

QStringList GraphPropertiesPanel::constValuesChanged(void) const
{
    QStringList changes;
    const auto globalNames = _graphEditor->listGlobals();

    //look for added constants
    for (const auto &name : globalNames)
    {
        if (std::find(_originalConstNames.begin(), _originalConstNames.end(), name) == _originalConstNames.end())
        {
            changes.push_back(tr("Created %1").arg(name));
        }
    }

    //look for removed constants
    for (const auto &name : _originalConstNames)
    {
        if (std::find(globalNames.begin(), globalNames.end(), name) == globalNames.end())
        {
            changes.push_back(tr("Removed %1").arg(name));
        }
        else if (_constNameToOriginal.at(name) != _graphEditor->getGlobalExpression(name))
        {
            changes.push_back(tr("Changed %1").arg(name));
        }
    }

    return changes;
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
    const auto globalNames = _graphEditor->listGlobals();
    if (std::find(globalNames.begin(), globalNames.end(), name) != globalNames.end())
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
    _graphEditor->setGlobalExpression(name, "0");
    this->updateConstantForm(name);
}

void GraphPropertiesPanel::updateAllConstantForms(void)
{
    for (const auto &name : _graphEditor->listGlobals())
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
        auto editWidget = this->makePropertyEditWidget();
        auto editLayout = new QVBoxLayout();
        editLayout->addWidget(editWidget);
        editLayout->addWidget(errorLabel);
        _constNameFormLayout->addRow(formLabel, editLayout);

        connect(editWidget, SIGNAL(widgetChanged(void)), this, SLOT(updateAllConstantForms(void)));
        connect(editWidget, SIGNAL(entryChanged(void)), this, SLOT(updateAllConstantForms(void)));
        connect(editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleCommit(void)));
        QMetaObject::invokeMethod(editWidget, "setValue", Qt::DirectConnection, Q_ARG(QString, _graphEditor->getGlobalExpression(name)));

        _constNameToFormLabel[name] = formLabel;
        _constNameToErrorLabel[name] = errorLabel;
        _constNameToEditWidget[name] = editWidget;
    }

    //update the widgets for this constant
    _constNameToFormLabel[name]->setText(name);
    //TODO
    auto editWidget = _constNameToEditWidget[name];
    QString entryValue; QMetaObject::invokeMethod(editWidget, "value", Qt::DirectConnection, Q_RETURN_ARG(QString, entryValue));
    _graphEditor->setGlobalExpression(name, entryValue);
}

QWidget *GraphPropertiesPanel::makePropertyEditWidget(const std::string &widgetType)
{
    //lookup the plugin to get the entry widget factory
    const auto plugin = Pothos::PluginRegistry::get(Pothos::PluginPath("/gui/EntryWidgets").join(widgetType));
    const auto &factory = plugin.getObject().extract<Pothos::Callable>();
    const Poco::JSON::Object::Ptr paramDesc(new Poco::JSON::Object());
    auto editWidget = factory.call<QWidget *>(paramDesc, static_cast<QWidget *>(this));
    editWidget->setLocale(QLocale::C);
    editWidget->setObjectName("BlockPropertiesEditWidget"); //style-sheet id name
    return editWidget;
}
