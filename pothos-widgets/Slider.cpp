// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QSlider>

/***********************************************************************
 * |PothosDoc Slider
 *
 * A slider widget for graphical control of an integer value.
 *
 * |category /Widgets
 * |keywords slider
 *
 * |param orientation The slider orientation (horizontal or veritical).
 * |default "HORIZONTAL"
 * |option [Horizontal] "HORIZONTAL"
 * |option [Veritical] "VERTICAL"
 * |preview disable
 *
 * |param value The initial value of this slider.
 * |default 0
 * |widget SpinBox()
 *
 * |param minimum The minimum integer value of this slider.
 * |default 0
 * |widget SpinBox()
 *
 * |param maximum The maximum integer value of this slider.
 * |default 100
 * |widget SpinBox()
 *
 * |mode displayWidget
 * |factory /widgets/slider(orientation)
 * |setter setValue(value)
 * |setter setMinimum(minimum)
 * |setter setMaximum(maximum)
 **********************************************************************/
class Slider : public QSlider, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const std::string &orientation)
    {
        return new Slider(orientation);
    }

    Slider(const std::string &orientation):
        QSlider((orientation == "HORIZONTAL")? Qt::Horizontal : Qt::Vertical)
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMaximum));
        this->registerSignal("valueChanged");
        connect(this, SIGNAL(valueChanged(const int)), this, SLOT(handleValueChanged(const int)));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void activate(void)
    {
        //emit current value when design becomes active
        this->emitSignal("valueChanged", this->value());
    }

private slots:
    void handleValueChanged(const int value)
    {
        this->emitSignal("valueChanged", value);
    }
};

static Pothos::BlockRegistry registerAdd(
    "/widgets/slider", &Slider::make);

#include "Slider.moc"
