// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Object.h>
#include <QSpinBox>
#include <limits>

/***********************************************************************
 * SpinBox for integer entry
 **********************************************************************/
class SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    SpinBox(QWidget *parent):
        QSpinBox(parent)
    {
        connect(this, SIGNAL(editingFinished(void)), this, SIGNAL(widgetChanged(void)));
        connect(this, SIGNAL(valueChanged(const QString &)), this, SLOT(handleWidgetChanged(const QString &)));
    }

public slots:
    QString value(void) const
    {
        return QSpinBox::text();
    }

    void setValue(const QString &value)
    {
        QSpinBox::setValue(value.toInt());
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
static QWidget *makeSpinBox(const Poco::JSON::Object::Ptr &paramDesc, QWidget *parent)
{
    Poco::JSON::Object::Ptr widgetKwargs(new Poco::JSON::Object());
    if (paramDesc->has("widgetKwargs")) widgetKwargs = paramDesc->getObject("widgetKwargs");

    auto spinBox = new SpinBox(parent);
    spinBox->setMinimum(widgetKwargs->optValue<int>("minimum", std::numeric_limits<int>::min()));
    spinBox->setMaximum(widgetKwargs->optValue<int>("maximum", std::numeric_limits<int>::max()));
    return spinBox;
}

pothos_static_block(registerSpinBox)
{
    Pothos::PluginRegistry::add("/gui/EntryWidgets/SpinBox", Pothos::Callable(&makeSpinBox));
}

#include "SpinBox.moc"
