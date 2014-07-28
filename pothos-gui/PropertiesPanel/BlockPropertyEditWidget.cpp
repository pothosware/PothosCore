// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //type to color
#include "PropertiesPanel/BlockPropertyEditWidget.hpp"
#include <QComboBox>
#include <QLineEdit>

BlockPropertyEditWidget::BlockPropertyEditWidget(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent):
    QStackedWidget(parent),
    _edit(nullptr)
{
    if (paramDesc->isArray("options"))
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
    else
    {
        auto lineEdit = new QLineEdit(this);
        connect(lineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(handleEditWidgetChanged(const QString &)));
        connect(lineEdit, SIGNAL(returnPressed(void)), this, SIGNAL(commitRequested(void)));
        _edit = lineEdit;
    }

    this->addWidget(_edit);
}

void BlockPropertyEditWidget::setValue(const QString &value)
{
    auto comboBox = dynamic_cast<QComboBox *>(_edit);
    if (comboBox != nullptr) for (int i = 0; i < comboBox->count(); i++)
    {
        if (comboBox->itemData(i).toString() == value) comboBox->setCurrentIndex(i);
    }
    auto lineEdit = dynamic_cast<QLineEdit *>(_edit);
    if (lineEdit != nullptr) lineEdit->setText(value);
}

QString BlockPropertyEditWidget::value(void) const
{
    QString newValue;
    auto comboBox = dynamic_cast<QComboBox *>(_edit);
    if (comboBox != nullptr) newValue = comboBox->itemData(comboBox->currentIndex()).toString();
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
        "QLineEdit{background:%1;color:%2;}")
        .arg(typeColor.name())
        .arg((typeColor.lightnessF() > 0.5)?"black":"white")
    );
}

void BlockPropertyEditWidget::handleEditWidgetChanged(const QString &)
{
    emit this->valueChanged();
}

