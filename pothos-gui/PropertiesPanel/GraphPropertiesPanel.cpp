// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //make icon theme
#include "PropertyEditWidget.hpp"
#include "GraphPropertiesPanel.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <Pothos/Proxy.hpp>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QToolTip>
#include <iostream>

GraphPropertiesPanel::GraphPropertiesPanel(GraphEditor *editor, QWidget *parent):
    QWidget(parent),
    _graphEditor(editor),
    _formLayout(new QFormLayout(this)),
    _constantNameEntry(nullptr),
    _constNameFormLayout(nullptr),
    _constRemovalMapper(new QSignalMapper(this))
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
        connect(_constRemovalMapper, SIGNAL(mapped(const QString &)), this, SLOT(handleConstRemoval(const QString &)));
        nameEntryLayout->addWidget(_constantNameEntry);
        nameEntryLayout->addWidget(nameEntryButton);

        constantsLayout->addLayout(_constNameFormLayout);
        constantsLayout->addLayout(nameEntryLayout);

        _formLayout->addRow(constantsBox);
    }

    //create widgets and make a backup of constants
    _originalConstNames = _graphEditor->listGlobals();
    for (const auto &name : _originalConstNames)
    {
        this->createConstantEditWidget(name);
        _constNameToOriginal[name] = _graphEditor->getGlobalExpression(name);
    }

    //cause the generation of entry forms for existing constants
    this->updateAllConstantForms();
}

void GraphPropertiesPanel::handleCancel(void)
{
    //widget cancel
    for (const auto &editWidget : _constNameToEditWidget)
    {
        editWidget.second->cancelEvents();
    }

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
            changes.push_back(tr("Created constant %1").arg(name));
        }
    }

    //look for removed constants
    for (const auto &name : _originalConstNames)
    {
        if (std::find(globalNames.begin(), globalNames.end(), name) == globalNames.end())
        {
            changes.push_back(tr("Removed constant %1").arg(name));
        }
        else if (_constNameToEditWidget.at(name)->changed())
        {
            changes.push_back(tr("Changed constant %1").arg(name));
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
    this->createConstantEditWidget(name);
    this->updateAllConstantForms();
}

void GraphPropertiesPanel::updateAllConstantForms(void)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("make");

    for (const auto &name : _graphEditor->listGlobals())
    {
        //update the widgets for this constant
        auto editWidget = _constNameToEditWidget[name];
        _graphEditor->setGlobalExpression(name, editWidget->value());

        try
        {
            const auto expr = _graphEditor->getGlobalExpression(name).toStdString();
            evalEnv.callProxy("registerConstant", name.toStdString(), expr);
            auto obj = evalEnv.callProxy("eval", name.toStdString());
            editWidget->setTypeStr(obj.call<std::string>("getTypeString"));
            editWidget->setErrorMsg(""); //clear errors
        }
        catch (const Pothos::Exception &ex)
        {
            editWidget->setErrorMsg(QString::fromStdString(ex.message()));
        }
    }

    _graphEditor->commitGlobalsChanges();
}

void GraphPropertiesPanel::createConstantEditWidget(const QString &name)
{
    //create edit widget
    const Poco::JSON::Object::Ptr paramDesc(new Poco::JSON::Object());
    auto editWidget = new PropertyEditWidget(_graphEditor->getGlobalExpression(name), paramDesc, this);
    connect(editWidget, SIGNAL(widgetChanged(void)), this, SLOT(updateAllConstantForms(void)));
    //connect(editWidget, SIGNAL(entryChanged(void)), this, SLOT(updateAllConstantForms(void)));
    connect(editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleCommit(void)));
    _constNameToEditWidget[name] = editWidget;

    //create removal button
    auto removalButton = new QPushButton(makeIconFromTheme("list-remove"), tr("Remove"), this);
    _constRemovalMapper->setMapping(removalButton, name);
    connect(removalButton, SIGNAL(clicked(void)), _constRemovalMapper, SLOT(map(void)));
    auto editLayout = new QHBoxLayout();
    editLayout->addWidget(editWidget);
    editLayout->addWidget(removalButton);

    //install into form
    auto formLabel = editWidget->makeFormLabel(name, this);
    _constNameFormLayout->addRow(formLabel, editLayout);

    //objects to delete
    _constNameToObjects[name].push_back(removalButton);
    _constNameToObjects[name].push_back(editWidget);
    _constNameToObjects[name].push_back(editLayout);
    _constNameToObjects[name].push_back(formLabel);
}

void GraphPropertiesPanel::handleConstRemoval(const QString &name)
{
    //delete objects
    _constNameToEditWidget[name]->cancelEvents();
    for (auto obj : _constNameToObjects.at(name))
    {
        delete obj;
    }
    _constNameToObjects.erase(name);

    //remove from globals list
    QStringList globals = _graphEditor->listGlobals();
    globals.erase(globals.begin() + globals.indexOf(name));
    _graphEditor->clearGlobals();
    for (const auto &name_i : globals)
    {
        _graphEditor->setGlobalExpression(name_i, _constNameToEditWidget[name_i]->value());
    }

    //update
    this->updateAllConstantForms();
}
