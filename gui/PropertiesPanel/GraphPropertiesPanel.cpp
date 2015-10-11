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
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QToolTip>
#include <QAction>
#include <iostream>

GraphPropertiesPanel::GraphPropertiesPanel(GraphEditor *editor, QWidget *parent):
    QWidget(parent),
    _graphEditor(editor),
    _formLayout(makeFormLayout(this)),
    _varNameEntry(new QLineEdit(this)),
    _varsFormLayout(makeFormLayout()),
    _varsAddButton(new QToolButton(this)),
    _varsRemoveButton(new QToolButton(this)),
    _varsMoveUpButton(new QToolButton(this)),
    _varsMoveDownButton(new QToolButton(this)),
    _varsSelectionGroup(new QButtonGroup(this))
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
        //create layouts
        auto constantsBox = new QGroupBox(tr("Global Variables"), this);
        auto constantsLayout = new QVBoxLayout(constantsBox);
        _formLayout->addRow(constantsBox);
        auto nameEntryLayout = new QHBoxLayout();
        constantsLayout->addLayout(nameEntryLayout);
        constantsLayout->addLayout(_varsFormLayout);

        //setup widgets
        _varNameEntry->setPlaceholderText(tr("Enter a new variable name"));
        auto addAction = new QAction(makeIconFromTheme("list-add"), tr("Create"), this);
        auto removeAction = new QAction(makeIconFromTheme("list-remove"), tr("Remove"), this);
        _varsAddButton->setDefaultAction(addAction);
        _varsRemoveButton->setDefaultAction(removeAction);
        _varsMoveUpButton->setArrowType(Qt::UpArrow);
        _varsMoveDownButton->setArrowType(Qt::DownArrow);

        //add widgets to the entry layout
        nameEntryLayout->addWidget(_varNameEntry);
        nameEntryLayout->addWidget(_varsAddButton);
        nameEntryLayout->addWidget(_varsMoveUpButton);
        nameEntryLayout->addWidget(_varsMoveDownButton);
        nameEntryLayout->addWidget(_varsRemoveButton);

        //set tool tips
        _varsAddButton->setToolTip(tr("Create new global variable"));
        _varsRemoveButton->setToolTip(tr("Remove selected global variable"));
        _varsMoveUpButton->setToolTip(tr("Move selected global variable up"));
        _varsMoveDownButton->setToolTip(tr("Move selected global variable down"));

        //connect signals
        connect(_varsAddButton, SIGNAL(clicked(void)), this, SLOT(handleCreateVariable(void)));
        connect(_varNameEntry, SIGNAL(returnPressed(void)), this, SLOT(handleCreateVariable(void)));
        connect(_varsRemoveButton, SIGNAL(clicked(void)), this, SLOT(handleVariableRemoval(void)));
        connect(_varsMoveUpButton, SIGNAL(clicked(void)), this, SLOT(handleVariableMoveUp(void)));
        connect(_varsMoveDownButton, SIGNAL(clicked(void)), this, SLOT(handleVariableMoveDown(void)));
    }

    //create widgets and make a backup of constants
    _originalVariableNames = _graphEditor->listGlobals();
    for (const auto &name : _originalVariableNames)
    {
        this->createVariableEditWidget(name);
        _varNameToOriginal[name] = _graphEditor->getGlobalExpression(name);
    }

    //cause the generation of entry forms for existing constants
    this->updateAllVariableForms();
}

void GraphPropertiesPanel::handleCancel(void)
{
    //widget cancel
    for (const auto &formData : _varToFormData)
    {
        formData.second.editWidget->cancelEvents();
    }

    //revert values
    _graphEditor->clearGlobals();
    for (const auto &name : _originalVariableNames)
    {
        _graphEditor->setGlobalExpression(name, _varNameToOriginal.at(name));
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
    this->updateAllVariableForms();

    //look for changes
    const auto propertiesModified = this->getChangeDescList();
    if (propertiesModified.empty()) return this->handleCancel();

    //emit state change
    auto desc = (propertiesModified.size() == 1)? propertiesModified.front() : tr("Modified graph properties");
    emit this->stateChanged(GraphState("document-properties", desc));

    //an edit widget return press signal may have us here,
    //and not the commit button, so make sure panel is deleted
    this->deleteLater();
}

QStringList GraphPropertiesPanel::getChangeDescList(void) const
{
    QStringList changes;
    const auto globalNames = _graphEditor->listGlobals();

    //look for reorder of variable list
    if (
        _originalVariableNames != _graphEditor->listGlobals() and
        QSet<QString>::fromList(_originalVariableNames) == QSet<QString>::fromList(_graphEditor->listGlobals())
    )
    {
        changes.push_back(tr("Reordered variables"));
    }

    //look for added constants
    for (const auto &name : globalNames)
    {
        if (std::find(_originalVariableNames.begin(), _originalVariableNames.end(), name) == _originalVariableNames.end())
        {
            changes.push_back(tr("Created variable %1").arg(name));
        }
    }

    //look for removed constants
    for (const auto &name : _originalVariableNames)
    {
        if (std::find(globalNames.begin(), globalNames.end(), name) == globalNames.end())
        {
            changes.push_back(tr("Removed variable %1").arg(name));
        }
        else if (_varToFormData.at(name).editWidget->changed())
        {
            changes.push_back(tr("Changed variable %1").arg(name));
        }
    }

    return changes;
}

void GraphPropertiesPanel::handleCreateVariable(void)
{
    const auto name = _varNameEntry->text().trimmed();
    _varNameEntry->setText(QString());

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
        errorMsg = tr("Variable '%1' already exists").arg(name);
    }

    //report error and exit
    if (not errorMsg.isEmpty())
    {
        QToolTip::showText(_varNameEntry->mapToGlobal(QPoint()), "<font color=\"red\">"+errorMsg+"</font>");
        return;
    }

    //success, add the form
    _graphEditor->setGlobalExpression(name, "0");
    this->createVariableEditWidget(name);
    this->updateAllVariableForms();
}

void GraphPropertiesPanel::updateAllVariableForms(void)
{
    //update the enables for mod buttons
    _varsRemoveButton->setEnabled(false);
    _varsMoveUpButton->setEnabled(false);
    _varsMoveDownButton->setEnabled(false);
    for (const auto &name : this->getSelectedVariables())
    {
        _varsRemoveButton->setEnabled(true);
        const int index = _graphEditor->listGlobals().indexOf(name);
        if (index != 0)
        {
            _varsMoveUpButton->setEnabled(true);
        }
        if (index+1 != _graphEditor->listGlobals().size())
        {
            _varsMoveDownButton->setEnabled(true);
        }
    }

    //clear the form layout so it can be recreated in order
    for (const auto &name : _graphEditor->listGlobals())
    {
        const auto &formData = _varToFormData.at(name);
        _varsFormLayout->removeWidget(formData.formLabel);
        _varsFormLayout->removeWidget(formData.formWidget);
    }

    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("make");

    for (const auto &name : _graphEditor->listGlobals())
    {
        //update the widgets for this constant
        const auto &formData = _varToFormData.at(name);
        auto editWidget = formData.editWidget;
        _graphEditor->setGlobalExpression(name, editWidget->value());
        _varsFormLayout->addRow(formData.formLabel, formData.formWidget);

        try
        {
            const auto expr = _graphEditor->getGlobalExpression(name).toStdString();
            evalEnv.callVoid("registerConstantExpr", name.toStdString(), expr);
            auto obj = evalEnv.callProxy("eval", name.toStdString());
            const auto typeStr = obj.call<std::string>("getTypeString");
            const auto toString = obj.call<std::string>("toString");
            editWidget->setTypeStr(typeStr);
            editWidget->setErrorMsg(""); //clear errors
            editWidget->setToolTip(QString::fromStdString(toString));
        }
        catch (const Pothos::Exception &ex)
        {
            editWidget->setErrorMsg(QString::fromStdString(ex.message()));
            editWidget->setToolTip(QString::fromStdString(ex.message()));
        }
    }

    _graphEditor->commitGlobalsChanges();
}

void GraphPropertiesPanel::createVariableEditWidget(const QString &name)
{
    auto &formData = _varToFormData[name];
    auto formWidget = new QWidget(this);
    auto editLayout = new QHBoxLayout(formWidget);
    editLayout->setContentsMargins(0, 0, 0, 0);

    //create edit widget
    const Poco::JSON::Object::Ptr paramDesc(new Poco::JSON::Object());
    auto editWidget = new PropertyEditWidget(_graphEditor->getGlobalExpression(name), paramDesc, this);
    connect(editWidget, SIGNAL(widgetChanged(void)), this, SLOT(updateAllVariableForms(void)));
    //connect(editWidget, SIGNAL(entryChanged(void)), this, SLOT(updateAllVariableForms(void)));
    connect(editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleCommit(void)));
    editLayout->addWidget(editWidget);

    //selection button
    auto radioButton = new QRadioButton(this);
    connect(radioButton, SIGNAL(clicked(void)), this, SLOT(updateAllVariableForms(void)));
    editLayout->addWidget(radioButton);
    _varsSelectionGroup->addButton(radioButton);

    //install into form
    formData.editWidget = editWidget;
    formData.formLabel = editWidget->makeFormLabel(name, this);
    formData.formWidget = formWidget;
    formData.radioButton = radioButton;
}

QStringList GraphPropertiesPanel::getSelectedVariables(void) const
{
    QStringList names;
    for (const auto &pair : _varToFormData)
    {
        if (pair.second.radioButton->isChecked()) names.push_back(pair.first);
    }
    return names;
}

void GraphPropertiesPanel::handleVariableRemoval(void)
{
    for (const auto &name : this->getSelectedVariables())
    {
        this->handleVariableRemoval(name);
    }
}

void GraphPropertiesPanel::handleVariableRemoval(const QString &name)
{
    //delete objects
    const auto &formData = _varToFormData.at(name);
    formData.editWidget->cancelEvents();
    delete formData.formWidget;
    _varToFormData.erase(name);

    //remove from globals list
    QStringList globals = _graphEditor->listGlobals();
    globals.erase(globals.begin() + globals.indexOf(name));
    _graphEditor->reorderGlobals(globals);

    //update
    this->updateAllVariableForms();
}

void GraphPropertiesPanel::handleVariableMoveUp(void)
{
    for (const auto &name : this->getSelectedVariables())
    {
        this->handleVariableMoveUp(name);
    }
}

void GraphPropertiesPanel::handleVariableMoveUp(const QString &name)
{
    QStringList globals = _graphEditor->listGlobals();
    const int index = globals.indexOf(name);
    if (index < 1) return; //ignore top most and -1 (not found)

    //move it up by swapping
    std::swap(globals[index-1], globals[index]);
    _graphEditor->reorderGlobals(globals);

    //update
    this->updateAllVariableForms();
}

void GraphPropertiesPanel::handleVariableMoveDown(void)
{
    for (const auto &name : this->getSelectedVariables())
    {
        this->handleVariableMoveDown(name);
    }
}

void GraphPropertiesPanel::handleVariableMoveDown(const QString &name)
{
    QStringList globals = _graphEditor->listGlobals();
    const int index = globals.indexOf(name);
    if (index == -1 or index+1 >= globals.size()) return; //ignore bottom most and -1 (not found)

    //move it down by swapping
    std::swap(globals[index+1], globals[index]);
    _graphEditor->reorderGlobals(globals);

    //update
    this->updateAllVariableForms();
}
