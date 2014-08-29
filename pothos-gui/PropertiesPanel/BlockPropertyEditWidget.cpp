// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PropertiesPanel/BlockPropertyEditWidget.hpp"
#include "ColorUtils/ColorUtils.hpp"
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <Poco/Logger.h>
#include <Poco/Exception.h>
#include <limits>

/***********************************************************************
 * Custom widget for string entry -- no quotes
 **********************************************************************/
class StringEntry : public QLineEdit
{
    Q_OBJECT
public:
    StringEntry(QWidget *parent):
        QLineEdit(parent){}

    QString value(void) const
    {
        auto s = this->text();
        return QString("\"%1\"").arg(s.replace("\"", "\\\"")); //escape
    }

    void setValue(const QString &s)
    {
        if (s.startsWith("\"") and s.endsWith("\""))
        {
            auto s0 = s.midRef(1, s.size()-2).toString();
            this->setText(s0.replace("\\\"", "\"")); //unescape
        }
        else this->setText(s);
    }
};

/***********************************************************************
 * BlockPropertyEditWidget implementation
 **********************************************************************/
BlockPropertyEditWidget::BlockPropertyEditWidget(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent):
    QStackedWidget(parent),
    _edit(nullptr)
{
    //extract widget args
    Poco::JSON::Array::Ptr widgetArgs(new Poco::JSON::Array());
    if (paramDesc->has("widgetArgs")) widgetArgs = paramDesc->getArray("widgetArgs");
    Poco::JSON::Object::Ptr widgetKwargs(new Poco::JSON::Object());
    if (paramDesc->has("widgetKwargs")) widgetKwargs = paramDesc->getObject("widgetKwargs");

    //extract widget type
    auto widgetType = paramDesc->optValue<std::string>("widgetType", "LineEdit");
    if (paramDesc->isArray("options")) widgetType = "ComboBox";

    if (widgetType == "ComboBox")
    {
        auto comboBox = new QComboBox(this);
        comboBox->setEditable(widgetKwargs->optValue<bool>("editable", false));
        for (const auto &optionObj : *paramDesc->getArray("options"))
        {
            const auto option = optionObj.extract<Poco::JSON::Object::Ptr>();
            comboBox->addItem(
                QString::fromStdString(option->getValue<std::string>("name")),
                QString::fromStdString(option->getValue<std::string>("value")));
        }
        connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        connect(comboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        _edit = comboBox;
    }
    else if (widgetType == "SpinBox")
    {
        auto spinBox = new QSpinBox(this);
        spinBox->setMinimum(widgetKwargs->optValue<int>("minimum", std::numeric_limits<int>::min()));
        spinBox->setMaximum(widgetKwargs->optValue<int>("maximum", std::numeric_limits<int>::max()));
        connect(spinBox, SIGNAL(editingFinished(void)), this, SLOT(handleEditWidgetChanged(void)));
        _edit = spinBox;
    }
    else if (widgetType == "DoubleSpinBox")
    {
        auto spinBox = new QDoubleSpinBox(this);
        spinBox->setMinimum(widgetKwargs->optValue<double>("minimum", -1e12));
        spinBox->setMaximum(widgetKwargs->optValue<double>("maximum", +1e12));
        spinBox->setSingleStep(widgetKwargs->optValue<double>("step", 0.01));
        spinBox->setDecimals(widgetKwargs->optValue<int>("decimals", 2));
        connect(spinBox, SIGNAL(editingFinished(void)), this, SLOT(handleEditWidgetChanged(void)));
        _edit = spinBox;
    }
    else if (widgetType == "StringEntry")
    {
        auto entry = new StringEntry(this);
        connect(entry, SIGNAL(textEdited(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        connect(entry, SIGNAL(returnPressed(void)), this, SIGNAL(commitRequested(void)));
        _edit = entry;
    }
    else
    {
        if (widgetType != "LineEdit")
        {
            poco_warning_f2(Poco::Logger::get("PothosGui.BlockPropertyEditWidget"),
                "unknown widget type '%s' for %s, defaulting to 'LineEdit'",
                widgetType, paramDesc->getValue<std::string>("name"));
        }
        auto lineEdit = new QLineEdit(this);
        connect(lineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        connect(lineEdit, SIGNAL(returnPressed(void)), this, SIGNAL(commitRequested(void)));
        _edit = lineEdit;
    }

    this->addWidget(_edit);
}

void BlockPropertyEditWidget::setValue(const QString &value)
{
    if (this->value() == value) return;

    auto comboBox = dynamic_cast<QComboBox *>(_edit);
    if (comboBox != nullptr)
    {
        int index = -1;
        for (int i = 0; i < comboBox->count(); i++)
        {
            if (comboBox->itemData(i).toString() == value) index = i;
        }
        if (index < 0) comboBox->setEditText(value);
        else comboBox->setCurrentIndex(index);
        return;
    }

    auto spinBox = dynamic_cast<QSpinBox *>(_edit);
    if (spinBox != nullptr) return spinBox->setValue(value.toInt());

    auto dSpinBox = dynamic_cast<QDoubleSpinBox *>(_edit);
    if (dSpinBox != nullptr) return dSpinBox->setValue(value.toDouble());

    auto strEntry = dynamic_cast<StringEntry *>(_edit);
    if (strEntry != nullptr) return strEntry->setValue(value);

    auto lineEdit = dynamic_cast<QLineEdit *>(_edit);
    if (lineEdit != nullptr) return lineEdit->setText(value);

    poco_bugcheck_msg("unknown widget");
}

QString BlockPropertyEditWidget::value(void) const
{
    auto comboBox = dynamic_cast<QComboBox *>(_edit);
    if (comboBox != nullptr)
    {
        const auto index = comboBox->currentIndex();
        if (index < 0 or comboBox->currentText() != comboBox->itemText(index)) return comboBox->currentText();
        else return comboBox->itemData(index).toString();
    }

    auto spinBox = dynamic_cast<QSpinBox *>(_edit);
    if (spinBox != nullptr) return QString("%1").arg(spinBox->value());

    auto dSpinBox = dynamic_cast<QDoubleSpinBox *>(_edit);
    if (dSpinBox != nullptr) return QString("%1").arg(dSpinBox->value());

    auto strEntry = dynamic_cast<StringEntry *>(_edit);
    if (strEntry != nullptr) return strEntry->value();

    auto lineEdit = dynamic_cast<QLineEdit *>(_edit);
    if (lineEdit != nullptr) return lineEdit->text();

    poco_bugcheck_msg("unknown widget");
    return "";
}

void BlockPropertyEditWidget::setColors(const std::string &typeStr)
{
    auto typeColor = typeStrToColor(typeStr);

    //set the fg and bg colors on the edit widget
    _edit->setObjectName("MyEditWidget");
    _edit->setStyleSheet(QString("#MyEditWidget{background:%1;color:%2;}")
        .arg(typeColor.name())
        .arg((typeColor.lightnessF() > 0.5)?"black":"white")
    );
}

void BlockPropertyEditWidget::handleEditWidgetChanged(const QString &)
{
    emit this->valueChanged();
}

void BlockPropertyEditWidget::handleEditWidgetChanged(void)
{
    emit this->valueChanged();
}

#include "BlockPropertyEditWidget.moc"
