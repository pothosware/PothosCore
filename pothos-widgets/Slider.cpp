// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include "MyDoubleSlider.hpp"

/***********************************************************************
 * |PothosDoc Slider
 *
 * A slider widget for graphical control of an integer value.
 *
 * |category /Widgets
 * |keywords slider
 *
 * |param orientation The slider orientation (horizontal or veritical).
 * |default "Horizontal"
 * |option [Horizontal] "Horizontal"
 * |option [Veritical] "Veritical"
 * |preview disable
 *
 * |param value The initial value of this slider.
 * |default 0.0
 *
 * |param minimum The minimum value of this slider.
 * |default -1.0
 *
 * |param maximum The maximum value of this slider.
 * |default +1.0
 *
 * |param step [Step Size] The increment between discrete values.
 * |default 0.01
 *
 * |mode graphWidget
 * |factory /widgets/slider(orientation)
 * |setter setValue(value)
 * |setter setMinimum(minimum)
 * |setter setMaximum(maximum)
 * |setter setSingleStep(step)
 **********************************************************************/
class Slider : public MyDoubleSlider, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const std::string &orientation)
    {
        return new Slider(orientation);
    }

    Slider(const std::string &orientation):
        MyDoubleSlider((orientation == "Horizontal")? Qt::Horizontal : Qt::Vertical)
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMaximum));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setSingleStep));
        this->registerSignal("valueChanged");
        connect(this, SIGNAL(valueChanged(const double)), this, SLOT(handleValueChanged(const double)));
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
    void handleValueChanged(const double value)
    {
        this->emitSignal("valueChanged", value);
    }
};

static Pothos::BlockRegistry registerSlider(
    "/widgets/slider", &Slider::make);

#include "Slider.moc"
