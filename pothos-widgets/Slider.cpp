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
 * |param value The initial value of this slider.
 * |default 0
 *
 * |param minimum The minimum integer value of this slider.
 * |default 0
 *
 * |param maximum The maximum integer value of this slider.
 * |default 100
 *
 * |factory /widgets/slider()
 * |setter setValue(value)
 * |setter setMinimum(minimum)
 * |setter setMaximum(maximum)
 **********************************************************************/
class Slider : public QSlider, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new Slider();
    }

    Slider(void)
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, getWidget));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMaximum));
        this->registerSignal("valueChanged");
        connect(this, SIGNAL(valueChanged(const int)), this, SLOT(handleValueChanged(const int)));
    }

    QWidget *getWidget(void)
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
