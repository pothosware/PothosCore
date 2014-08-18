// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include "MyDoubleSlider.hpp"
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

/***********************************************************************
 * |PothosDoc Numeric Entry
 *
 * The numeric entry widget display's a numeric value in a text-entry box
 * with increment and decrement buttons.
 *
 * |category /Widgets
 * |keywords numeric entry spinbox
 *
 * |param title The name of the value displayed by this widget
 * |default "My Numeric Value"
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
 * |widget SpinBox(minimum=0, maximum=10)
 * |preview disable
 *
 * |param sliderVisible [Has Slider] Does this numeric entry have a slider control?
 * |default true
 * |option [Show Slider] true
 * |option [Hide Slider] false
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
 * |setter setSliderVisible(sliderVisible)
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
        _spinBox(new QDoubleSpinBox(this)),
        _slider(new MyDoubleSlider(Qt::Horizontal, this))
    {
        auto vlayout = new QVBoxLayout(this);
        auto hlayout = new QHBoxLayout();
        vlayout->setContentsMargins(QMargins());
        vlayout->setSpacing(0);
        vlayout->addLayout(hlayout);
        vlayout->addWidget(_slider);
        hlayout->setContentsMargins(QMargins());
        hlayout->setSpacing(1);
        hlayout->addWidget(_label);
        hlayout->addWidget(_spinBox);

        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setMaximum));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setDecimals));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setSingleStep));
        this->registerCall(this, POTHOS_FCN_TUPLE(NumericEntry, setSliderVisible));

        this->registerSignal("valueChanged");
        connect(_spinBox, SIGNAL(valueChanged(const double)), this, SLOT(handleValueChanged(const double)));
        connect(_slider, SIGNAL(valueChanged(const double)), this, SLOT(handleValueChanged(const double)));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void activate(void)
    {
        //emit current value when design becomes active
        this->emitSignal("valueChanged", _spinBox->value());
    }

    void setTitle(const QString &title)
    {
        const auto text = QString("<b>%1</b>").arg(title.toHtmlEscaped());
        //cannot call setText in calling thread, forward to the label slot
        QMetaObject::invokeMethod(_label, "setText", Qt::QueuedConnection, Q_ARG(QString, text));
    }

    void setValue(const double val)
    {
        _spinBox->setValue(val);
        _slider->setValue(val);
    }

    void setMinimum(const double min)
    {
        _spinBox->setMinimum(min);
        _slider->setMinimum(min);
    }

    void setMaximum(const double max)
    {
        _spinBox->setMaximum(max);
        _slider->setMaximum(max);
    }

    void setDecimals(const int prec)
    {
        _spinBox->setDecimals(prec);
    }

    void setSingleStep(const double val)
    {
        _spinBox->setSingleStep(val);
        _slider->setSingleStep(val);
    }

    void setSliderVisible(const bool visible)
    {
        _slider->setVisible(visible);
    }

private slots:
    void handleValueChanged(const double value)
    {
        _spinBox->blockSignals(true);
        _slider->blockSignals(true);
        this->setValue(value);
        _spinBox->blockSignals(false);
        _slider->blockSignals(false);

        this->emitSignal("valueChanged", value);
    }

private:
    QLabel *_label;
    QDoubleSpinBox *_spinBox;
    MyDoubleSlider *_slider;
};

static Pothos::BlockRegistry registerNumericEntry(
    "/widgets/numeric_entry", &NumericEntry::make);

#include "NumericEntry.moc"
