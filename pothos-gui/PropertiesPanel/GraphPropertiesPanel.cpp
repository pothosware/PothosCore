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
#include <QToolTip>
#include <iostream>

GraphPropertiesPanel::GraphPropertiesPanel(GraphEditor *editor, QWidget *parent):
    QWidget(parent),
    _graphEditor(editor),
    _formLayout(new QFormLayout(this)),
    _constantNameEntry(nullptr),
    _constantsFormLayout(nullptr),
    _constantsAddButton(new QPushButton(makeIconFromTheme("list-add"), tr("Create"), this)),
    _constantsRemoveButton(new QPushButton(makeIconFromTheme("list-remove"), tr("Remove"), this)),
    _constantsMoveUpButton(new QToolButton(this)),
    _constantsMoveDownButton(new QToolButton(this))
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
        _constantsFormLayout = new QFormLayout();

        auto nameEntryLayout = new QHBoxLayout();
        _constantNameEntry = new QLineEdit(constantsBox);
        _constantNameEntry->setPlaceholderText(tr("Enter a new constant name"));
        connect(_constantsAddButton, SIGNAL(clicked(void)), this, SLOT(handleCreateConstant(void)));
        connect(_constantNameEntry, SIGNAL(returnPressed(void)), this, SLOT(handleCreateConstant(void)));
        connect(_constantsRemoveButton, SIGNAL(clicked(void)), this, SLOT(handleConstRemoval(void)));
        connect(_constantsMoveUpButton, SIGNAL(clicked(void)), this, SLOT(handleConstMoveUp(void)));
        connect(_constantsMoveDownButton, SIGNAL(clicked(void)), this, SLOT(handleConstMoveDown(void)));
        nameEntryLayout->addWidget(_constantNameEntry);
        nameEntryLayout->addWidget(_constantsAddButton);

        _constantsMoveUpButton->setArrowType(Qt::UpArrow);
        _constantsMoveDownButton->setArrowType(Qt::DownArrow);
        nameEntryLayout->addWidget(_constantsMoveUpButton);
        nameEntryLayout->addWidget(_constantsMoveDownButton);
        nameEntryLayout->addWidget(_constantsRemoveButton);

        constantsLayout->addLayout(nameEntryLayout);
        constantsLayout->addLayout(_constantsFormLayout);

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
    for (const auto &formData : _constantToFormData)
    {
        formData.second.editWidget->cancelEvents();
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
        else if (_constantToFormData.at(name).editWidget->changed())
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
    //update the enables for mod buttons
    _constantsRemoveButton->setEnabled(false);
    _constantsMoveUpButton->setEnabled(false);
    _constantsMoveDownButton->setEnabled(false);
    for (const auto &name : this->getSelectedConstants())
    {
        _constantsRemoveButton->setEnabled(true);
        const int index = _graphEditor->listGlobals().indexOf(name);
        if (index != 0)
        {
            _constantsMoveUpButton->setEnabled(true);
        }
        if (index+1 != _graphEditor->listGlobals().size())
        {
            _constantsMoveDownButton->setEnabled(true);
        }
    }

    //clear the form layout so it can be recreated in order
    for (const auto &name : _graphEditor->listGlobals())
    {
        const auto &formData = _constantToFormData.at(name);
        _constantsFormLayout->removeWidget(formData.formLabel);
        _constantsFormLayout->removeItem(formData.editLayout);
    }

    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("make");

    for (const auto &name : _graphEditor->listGlobals())
    {
        //update the widgets for this constant
        const auto &formData = _constantToFormData.at(name);
        auto editWidget = formData.editWidget;
        _graphEditor->setGlobalExpression(name, editWidget->value());
        _constantsFormLayout->addRow(formData.formLabel, formData.editLayout);

        try
        {
            const auto expr = _graphEditor->getGlobalExpression(name).toStdString();
            evalEnv.callProxy("registerConstantExpr", name.toStdString(), expr);
            auto obj = evalEnv.callProxy("eval", name.toStdString());
            const auto typeStr = obj.call<std::string>("getTypeString");
            editWidget->setTypeStr(typeStr);
            editWidget->setErrorMsg(""); //clear errors
            editWidget->setToolTip(QString::fromStdString(typeStr).toHtmlEscaped());
        }
        catch (const Pothos::Exception &ex)
        {
            editWidget->setErrorMsg(QString::fromStdString(ex.message()));
            editWidget->setToolTip(QString::fromStdString(ex.message()));
        }
    }

    _graphEditor->commitGlobalsChanges();
}

void GraphPropertiesPanel::createConstantEditWidget(const QString &name)
{
    auto &formData = _constantToFormData[name];

    //create edit widget
    const Poco::JSON::Object::Ptr paramDesc(new Poco::JSON::Object());
    auto editWidget = new PropertyEditWidget(_graphEditor->getGlobalExpression(name), paramDesc, this);
    connect(editWidget, SIGNAL(widgetChanged(void)), this, SLOT(updateAllConstantForms(void)));
    //connect(editWidget, SIGNAL(entryChanged(void)), this, SLOT(updateAllConstantForms(void)));
    connect(editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleCommit(void)));
    formData.editWidget = editWidget;
    auto editLayout = new QHBoxLayout();
    editLayout->addWidget(editWidget);

    //selection button
    auto radioButton = new QRadioButton(this);
    connect(radioButton, SIGNAL(clicked(void)), this, SLOT(updateAllConstantForms(void)));
    formData.radioButton = radioButton;
    editLayout->addWidget(radioButton);

    //install into form
    formData.formLabel = editWidget->makeFormLabel(name, this);
    formData.editLayout = editLayout;
}

QStringList GraphPropertiesPanel::getSelectedConstants(void) const
{
    QStringList names;
    for (const auto &pair : _constantToFormData)
    {
        if (pair.second.radioButton->isChecked()) names.push_back(pair.first);
    }
    return names;
}

void GraphPropertiesPanel::handleConstRemoval(void)
{
    for (const auto &name : this->getSelectedConstants())
    {
        this->handleConstRemoval(name);
    }
}

void GraphPropertiesPanel::handleConstRemoval(const QString &name)
{
    //delete objects
    const auto &formData = _constantToFormData.at(name);
    formData.editWidget->cancelEvents();
    delete formData.formLabel;
    delete formData.editWidget;
    delete formData.radioButton;
    delete formData.editLayout;
    _constantToFormData.erase(name);

    //remove from globals list
    QStringList globals = _graphEditor->listGlobals();
    globals.erase(globals.begin() + globals.indexOf(name));
    _graphEditor->reorderGlobals(globals);

    //update
    this->updateAllConstantForms();
}

void GraphPropertiesPanel::handleConstMoveUp(void)
{
    for (const auto &name : this->getSelectedConstants())
    {
        this->handleConstMoveUp(name);
    }
}

void GraphPropertiesPanel::handleConstMoveUp(const QString &name)
{
    QStringList globals = _graphEditor->listGlobals();
    const int index = globals.indexOf(name);
    if (index < 1) return; //ignore top most and -1 (not found)

    //move it up by swapping
    std::swap(globals[index-1], globals[index]);
    _graphEditor->reorderGlobals(globals);

    //update
    this->updateAllConstantForms();
}

void GraphPropertiesPanel::handleConstMoveDown(void)
{
    for (const auto &name : this->getSelectedConstants())
    {
        this->handleConstMoveDown(name);
    }
}

void GraphPropertiesPanel::handleConstMoveDown(const QString &name)
{
    QStringList globals = _graphEditor->listGlobals();
    const int index = globals.indexOf(name);
    if (index == -1 or index+1 >= globals.size()) return; //ignore bottom most and -1 (not found)

    //move it down by swapping
    std::swap(globals[index+1], globals[index]);
    _graphEditor->reorderGlobals(globals);

    //update
    this->updateAllConstantForms();
}
