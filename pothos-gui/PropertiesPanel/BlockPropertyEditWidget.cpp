// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PropertiesPanel/BlockPropertyEditWidget.hpp"
#include "ColorUtils/ColorUtils.hpp"
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <Poco/Logger.h>
#include <limits>

BlockPropertyEditWidget::BlockPropertyEditWidget(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent):
    QStackedWidget(parent),
    _edit(nullptr)
{
    std::string widgetType;
    if (paramDesc->has("widget")) widgetType = paramDesc->getValue<std::string>("widget");
    else if (paramDesc->isArray("options")) widgetType = "ComboBox";
    else widgetType = "LineEdit";

    if (widgetType == "ComboBox")
    {
        auto comboBox = new QComboBox(this);
        _edit = comboBox;
        //combo->setEditable(true);
        for (const auto &optionObj : *paramDesc->getArray("options"))
        {
            const auto option = optionObj.extract<Poco::JSON::Object::Ptr>();
            comboBox->addItem(
                QString::fromStdString(option->getValue<std::string>("name")),
                QString::fromStdString(option->getValue<std::string>("value")));
        }
        connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
    }
    else if (widgetType == "SpinBox")
    {
        auto spinBox = new QSpinBox(this);
        spinBox->setMinimum(std::numeric_limits<int>::min());
        spinBox->setMaximum(std::numeric_limits<int>::max());
        _edit = spinBox;
        connect(spinBox, SIGNAL(editingFinished(void)), this, SLOT(handleEditWidgetChanged(void)));
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
    if (comboBox != nullptr) for (int i = 0; i < comboBox->count(); i++)
    {
        if (comboBox->itemData(i).toString() == value) comboBox->setCurrentIndex(i);
    }

    auto spinBox = dynamic_cast<QSpinBox *>(_edit);
    if (spinBox != nullptr) spinBox->setValue(value.toInt());

    auto lineEdit = dynamic_cast<QLineEdit *>(_edit);
    if (lineEdit != nullptr) lineEdit->setText(value);
}

QString BlockPropertyEditWidget::value(void) const
{
    QString newValue;

    auto comboBox = dynamic_cast<QComboBox *>(_edit);
    if (comboBox != nullptr) newValue = comboBox->itemData(comboBox->currentIndex()).toString();

    auto spinBox = dynamic_cast<QSpinBox *>(_edit);
    if (spinBox != nullptr) newValue = QString("%1").arg(spinBox->value());

    auto lineEdit = dynamic_cast<QLineEdit *>(_edit);
    if (lineEdit != nullptr) newValue = lineEdit->text();

    return newValue;
}

void BlockPropertyEditWidget::setColors(const std::string &typeStr)
{
    auto typeColor = typeStrToColor(typeStr);

    //set the fg and bg colors for all possible widget types
    _edit->setStyleSheet(QString(
        "QComboBox{background:%1;color:%2;}"
        "QSpinBox{background:%1;color:%2;}"
        "QLineEdit{background:%1;color:%2;}")
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

