// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PropertyEditWidget.hpp"
#include "ColorUtils/ColorUtils.hpp"
#include <QLabel>
#include <QLocale>
#include <QTimer>
#include <QVBoxLayout>
#include <Pothos/Plugin.hpp>
#include <Poco/Logger.h>

/*!
 * We could remove the timer with the eval-background system.
 * But rather, it may still be useful to have an idle period
 * in which we accept new edit events before submitting changes.
 * So just leave this as a small number for the time-being.
 */
static const long UPDATE_TIMER_MS = 500;

PropertyEditWidget::PropertyEditWidget(const QString &initialValue, const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent):
    _initialValue(initialValue),
    _editWidget(nullptr),
    _errorLabel(new QLabel(this)),
    _formLabel(nullptr),
    _unitsStr(QString::fromStdString(paramDesc->optValue<std::string>("units", ""))),
    _entryTimer(new QTimer(this))
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

    //signals to internal handler
    connect(_editWidget, SIGNAL(widgetChanged(void)), this, SLOT(handleWidgetChanged(void)));
    connect(_editWidget, SIGNAL(entryChanged(void)), this, SLOT(handleEntryChanged(void)));
    connect(_editWidget, SIGNAL(commitRequested(void)), this, SLOT(handleCommitRequested(void)));

    //setup entry timer - timeout acts like widget changed
    _entryTimer->setSingleShot(true);
    _entryTimer->setInterval(UPDATE_TIMER_MS);
    connect(_entryTimer, SIGNAL(timeout(void)), this, SIGNAL(widgetChanged(void)));

    //layout internal widgets
    auto editLayout = new QVBoxLayout(this);
    editLayout->setContentsMargins(0, 0, 0, 0);
    editLayout->addWidget(_editWidget);
    editLayout->addWidget(_errorLabel);

    //update display
    this->updateInternals();
}

PropertyEditWidget::~PropertyEditWidget(void)
{
    //we dont own form label, so it has to be explicitly deleted
    delete _formLabel;
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
    this->setBackgroundColor(typeStrToColor(typeStr));
}

void PropertyEditWidget::setErrorMsg(const QString &errorMsg)
{
    _errorMsg = errorMsg;
    this->updateInternals();
}

void PropertyEditWidget::setBackgroundColor(const QColor color)
{
    _editWidget->setStyleSheet(QString("#BlockPropertiesEditWidget{background:%1;color:%2;}")
        .arg(color.name()).arg((color.lightnessF() > 0.5)?"black":"white"));
}

QLabel *PropertyEditWidget::makeFormLabel(const QString &text, QWidget *parent)
{
    if (not _formLabel)
    {
        _formLabelText = text;
        _formLabel = new QLabel(text, parent);
        this->updateInternals();
    }
    return _formLabel;
}

void PropertyEditWidget::updateInternals(void)
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
    if (_formLabel) _formLabel->setText(formLabelText);
}

void PropertyEditWidget::handleWidgetChanged(void)
{
    this->updateInternals();
    emit this->widgetChanged();
}

void PropertyEditWidget::handleEntryChanged(void)
{
    _entryTimer->start(UPDATE_TIMER_MS);
    this->updateInternals();
    emit this->entryChanged();
}

void PropertyEditWidget::handleCommitRequested(void)
{
    this->flushEvents();
    this->updateInternals();
    emit this->commitRequested();
}

void PropertyEditWidget::cancelEvents(void)
{
    _entryTimer->stop();
}

void PropertyEditWidget::flushEvents(void)
{
    if (not _entryTimer->isActive()) return;
    _entryTimer->stop();
    this->handleEntryChanged();
}
