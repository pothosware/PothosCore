// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PropertyEditWidget.hpp"
#include "ColorUtils/ColorUtils.hpp"
#include <QLabel>
#include <QLocale>
#include <QVBoxLayout>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>

PropertyEditWidget::PropertyEditWidget(const QString &initialValue, const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent):
    _initialValue(initialValue),
    _editWidget(nullptr),
    _errorLabel(new QLabel(this)),
    _formLabel(nullptr),
    _unitsStr(QString::fromStdString(paramDesc->optValue<std::string>("units", "")))
{
    //extract widget type
    auto widgetType = paramDesc->optValue<std::string>("widgetType", "LineEdit");
    if (paramDesc->isArray("options")) widgetType = "ComboBox";
    if (widgetType.empty()) widgetType = "LineEdit";

    //check if the widget type exists in the plugin tree
    if (not Pothos::PluginRegistry::exists(Pothos::PluginPath("/gui/EntryWidgets").join(widgetType)))
    {
        poco_error_f1(Poco::Logger::get("PothosGui.BlockPropertiesPanel"), "widget type %s does not exist", widgetType);
        widgetType = "LineEdit";
    }

    //lookup the plugin to get the entry widget factory
    const auto plugin = Pothos::PluginRegistry::get(Pothos::PluginPath("/gui/EntryWidgets").join(widgetType));
    const auto &factory = plugin.getObject().extract<Pothos::Callable>();
    _editWidget = factory.call<QWidget *>(paramDesc, static_cast<QWidget *>(parent));
    _editWidget->setLocale(QLocale::C);
    _editWidget->setObjectName("BlockPropertiesEditWidget"); //style-sheet id name

    //initialize value
    this->setValue(initialValue);

    //signals to top level
    connect(_editWidget, SIGNAL(widgetChanged(void)), this, SIGNAL(widgetChanged(void)));
    connect(_editWidget, SIGNAL(entryChanged(void)), this, SIGNAL(entryChanged(void)));
    connect(_editWidget, SIGNAL(commitRequested(void)), this, SIGNAL(commitRequested(void)));

    //signals to internal handler
    connect(_editWidget, SIGNAL(widgetChanged(void)), this, SLOT(handleInternalChange(void)));
    connect(_editWidget, SIGNAL(entryChanged(void)), this, SLOT(handleInternalChange(void)));
    connect(_editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleInternalChange(void)));

    //layout internal widgets
    auto editLayout = new QVBoxLayout(this);
    editLayout->setContentsMargins(0, 0, 0, 0);
    editLayout->addWidget(_editWidget);
    editLayout->addWidget(_errorLabel);

    //update display
    this->handleInternalChange();
}

const QString &PropertyEditWidget::initialValue(void) const
{
    return _initialValue;
}

bool PropertyEditWidget::changed(void) const
{
    return this->value() != this->initialValue();
}

QString PropertyEditWidget::value(void) const
{
    QString value;
    QMetaObject::invokeMethod(_editWidget, "value", Qt::DirectConnection, Q_RETURN_ARG(QString, value));
    return value;
}

void PropertyEditWidget::setValue(const QString &value)
{
    QMetaObject::invokeMethod(_editWidget, "setValue", Qt::DirectConnection, Q_ARG(QString, value));
}

void PropertyEditWidget::setTypeStr(const std::string &typeStr)
{
    const auto typeColor = typeStrToColor(typeStr);
    _editWidget->setStyleSheet(QString("#BlockPropertiesEditWidget{background:%1;color:%2;}")
        .arg(typeColor.name()).arg((typeColor.lightnessF() > 0.5)?"black":"white"));
}

void PropertyEditWidget::setErrorMsg(const QString &errorMsg)
{
    _errorMsg = errorMsg;
    this->handleInternalChange();
}

QLabel *PropertyEditWidget::makeFormLabel(const QString &text, QWidget *parent)
{
    _formLabelText = text;
    _formLabel = new QLabel(text, parent);
    this->handleInternalChange();
    return _formLabel;
}

void PropertyEditWidget::handleInternalChange(void)
{
    //determine state
    const bool hasError = not _errorMsg.isEmpty();
    const bool hasUnits = not _unitsStr.isEmpty();

    //update the error label
    _errorLabel->setVisible(hasError);
    _errorLabel->setText(QString("<span style='color:red;'><p><i>%1</i></p></span>").arg(_errorMsg.toHtmlEscaped()));
    _errorLabel->setWordWrap(true);

    //generate the form label
    auto formLabelText = QString("<span style='color:%1;'><b>%2%3</b></span>")
        .arg(hasError?"red":"black")
        .arg(_formLabelText)
        .arg(this->changed()?"*":"");
    if (hasUnits) formLabelText += QString("<br /><i>%1</i>").arg(_unitsStr);
    if (_formLabel != nullptr) _formLabel->setText(formLabelText);
}
