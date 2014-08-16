// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>

/***********************************************************************
 * |PothosDoc Numeric Entry
 *
 * The numeric entry widget display's a numeric value in a text-entry box
 * with increment and decrement buttons.
 *
 * |category /Widgets
 * |keywords nuymeric entry spinbox
 *
 * |param title The name of the value displayed by this widget
 * |default "MyValue"
 *
 * |param value The initial value of this entry.
 * |default 0.0
 *
 * |param minimum The minimum value of this entry.
 * |default -1.0
 *
 * |param maximum The maximum value of this entry.
 * |default +1.0
 *
 * |param step [Step Size] The increment between discrete values.
 * |default 0.01
 *
 * |param precision The number of decimal points to display
 * |default 2
 * |preview disable
 *
 * |mode graphWidget
 * |factory /widgets/numeric_entry()
 * |setter setTitle(title)
 * |setter setValue(value)
 * |setter setMinimum(minimum)
 * |setter setMaximum(maximum)
 * |setter setSingleStep(step)
 * |setter setDecimals(precision)
 **********************************************************************/
class NumericEntry : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new NumericEntry();
    }

    NumericEntry(void):
        _label(new QLabel(this)),
        _spinBox(new QDoubleSpinBox(this))
    {
        auto layout = new QHBoxLayout(this);
        layout->addWidget(_label);
        layout->addWidget(_spinBox);
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setMaximum));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setDecimals));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setSingleStep));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void setTitle(const QString &title)
    {
        _label->setText(QString("<b>%1</b>").arg(title.toHtmlEscaped()));
    }

    void setValue(const double val)
    {
        _spinBox->setValue(val);
    }

    void setMinimum(const double min)
    {
        _spinBox->setMinimum(min);
    }

    void setMaximum(const double max)
    {
        _spinBox->setMaximum(max);
    }

    void setDecimals(const int prec)
    {
        _spinBox->setDecimals(prec);
    }

    void setSingleStep(const double val)
    {
        _spinBox->setSingleStep(val);
    }

private:
    QLabel *_label;
    QDoubleSpinBox *_spinBox;
};

static Pothos::BlockRegistry registerAdd(
    "/widgets/numeric_entry", &NumericEntry::make);

#include "NumericEntry.moc"
