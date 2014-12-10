// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Object.h>
#include <QComboBox>

/***********************************************************************
 * ComboBox for common data type entry
 **********************************************************************/
class ComboBox : public QComboBox
{
    Q_OBJECT
public:
    ComboBox(QWidget *parent):
        QComboBox(parent)
    {
        connect(this, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(handleWidgetChanged(const QString &)));
        connect(this, SIGNAL(editTextChanged(const QString &)), this, SLOT(handleEntryChanged(const QString &)));
    }

public slots:
    QString value(void) const
    {
        const auto index = QComboBox::currentIndex();
        if (index < 0 or QComboBox::currentText() != QComboBox::itemText(index)) return QComboBox::currentText();
        else return QComboBox::itemData(index).toString();
    }

    void setValue(const QString &value)
    {
        int index = -1;
        for (int i = 0; i < QComboBox::count(); i++)
        {
            if (QComboBox::itemData(i).toString() == value) index = i;
        }
        if (index < 0) QComboBox::setEditText(value);
        else QComboBox::setCurrentIndex(index);
    }

signals:
    void commitRequested(void);
    void widgetChanged(void);
    void entryChanged(void);

private slots:
    void handleWidgetChanged(const QString &)
    {
        emit this->widgetChanged();
    }
    void handleEntryChanged(const QString &)
    {
        emit this->entryChanged();
    }
};

/***********************************************************************
 * Factory function and registration
 **********************************************************************/
static QWidget *makeComboBox(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent)
{
    Poco::JSON::Object::Ptr widgetKwargs(new Poco::JSON::Object());
    if (paramDesc->has("widgetKwargs")) widgetKwargs = paramDesc->getObject("widgetKwargs");

    auto comboBox = new ComboBox(parent);
    comboBox->setEditable(widgetKwargs->optValue<bool>("editable", false));
    if (paramDesc->isArray("options")) for (const auto &optionObj : *paramDesc->getArray("options"))
    {
        const auto option = optionObj.extract<Poco::JSON::Object::Ptr>();
        comboBox->addItem(
            QString::fromStdString(option->getValue<std::string>("name")),
            QString::fromStdString(option->getValue<std::string>("value")));
    }
    return comboBox;
}

static QWidget *makeDTypeChooser(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent)
{
    Poco::JSON::Object::Ptr widgetKwargs(new Poco::JSON::Object());
    if (paramDesc->has("widgetKwargs")) widgetKwargs = paramDesc->getObject("widgetKwargs");

    auto comboBox = new ComboBox(parent);
    for (int mode = 0; mode <= 1; mode++)
    {
        const std::string keyPrefix((mode == 0)? "c":"");
        const QString namePrefix((mode == 0)? "Complex ":"");
        const QString aliasPrefix((mode == 0)? "complex_":"");
        for (int bytes = 64; bytes >= 32; bytes /= 2)
        {
            if (widgetKwargs->has(keyPrefix+"float")) comboBox->addItem(QString("%1Float%2").arg(namePrefix).arg(bytes), QString("\"%1float%2\"").arg(aliasPrefix).arg(bytes));
        }
        for (int bytes = 64; bytes >= 8; bytes /= 2)
        {
            if (widgetKwargs->has(keyPrefix+"int")) comboBox->addItem(QString("%1Int%2").arg(namePrefix).arg(bytes), QString("\"%1int%2\"").arg(aliasPrefix).arg(bytes));
            if (widgetKwargs->has(keyPrefix+"uint")) comboBox->addItem(QString("%1UInt%2").arg(namePrefix).arg(bytes), QString("\"%1uint%2\"").arg(aliasPrefix).arg(bytes));
        }
    }
    return comboBox;
}

pothos_static_block(registerComboBox)
{
    Pothos::PluginRegistry::add("/gui/EntryWidgets/ComboBox", Pothos::Callable(&makeComboBox));
    Pothos::PluginRegistry::add("/gui/EntryWidgets/DTypeChooser", Pothos::Callable(&makeDTypeChooser));
}

#include "ComboBox.moc"
