// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Object.h>
#include <QDoubleSpinBox>
#include <limits>

/***********************************************************************
 * DoubleSpinBox for floating point entry
 **********************************************************************/
class DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    DoubleSpinBox(QWidget *parent):
        QDoubleSpinBox(parent)
    {
        connect(this, SIGNAL(editingFinished(void)), this, SIGNAL(widgetChanged(void)));
        connect(this, SIGNAL(valueChanged(const QString &)), this, SLOT(handleWidgetChanged(const QString &)));
    }

public slots:
    QString value(void) const
    {
        return QDoubleSpinBox::text();
    }

    void setValue(const QString &value)
    {
        QDoubleSpinBox::setValue(value.toDouble());
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
};

/***********************************************************************
 * Factory function and registration
 **********************************************************************/
static QWidget *makeDoubleSpinBox(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent)
{
    Poco::JSON::Object::Ptr widgetKwargs(new Poco::JSON::Object());
    if (paramDesc->has("widgetKwargs")) widgetKwargs = paramDesc->getObject("widgetKwargs");

    auto spinBox = new DoubleSpinBox(parent);
    spinBox->setMinimum(widgetKwargs->optValue<double>("minimum", -1e12));
    spinBox->setMaximum(widgetKwargs->optValue<double>("maximum", +1e12));
    spinBox->setSingleStep(widgetKwargs->optValue<double>("step", 0.01));
    spinBox->setDecimals(widgetKwargs->optValue<int>("decimals", 2));
    return spinBox;
}

pothos_static_block(registerDoubleSpinBox)
{
    Pothos::PluginRegistry::add("/gui/EntryWidgets/DoubleSpinBox", Pothos::Callable(&makeDoubleSpinBox));
}

#include "DoubleSpinBox.moc"
