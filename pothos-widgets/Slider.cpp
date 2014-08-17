// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <QSlider>
#include <QHBoxLayout>

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
class Slider : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(const std::string &orientation)
    {
        return new Slider(orientation);
    }

    Slider(const std::string &orientation):
        _slider(new QSlider((orientation == "Horizontal")? Qt::Horizontal : Qt::Vertical, this))
    {
        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(QMargins());
        layout->setSpacing(0);
        layout->addWidget(_slider);

        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setMaximum));
        this->registerCall(this, POTHOS_FCN_TUPLE(Slider, setSingleStep));
        this->registerSignal("valueChanged");
        connect(_slider, SIGNAL(valueChanged(const int)), this, SLOT(handleValueChanged(const int)));
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

    double value(void) const
    {
        int int_norm = _slider->value()-_slider->minimum();
        double scale = int_norm/double(_slider->maximum()-_slider->minimum());
        double float_norm = scale*(_maximum - _minimum);
        return float_norm + _minimum;
    }

    void setValue(const double val)
    {
        double float_norm = val - _minimum;
        double scale = float_norm/(_maximum - _minimum);
        int int_norm = int(scale*(_slider->maximum()-_slider->minimum()));
        _slider->setValue(int_norm + _slider->minimum());
    }

    void setMinimum(const double min)
    {
        _minimum = min;
        this->updateRange();
    }

    void setMaximum(const double max)
    {
        _maximum = max;
        this->updateRange();
    }

    void setSingleStep(const double val)
    {
        _stepSize = val;
        this->updateRange();
    }

private slots:
    void handleValueChanged(const int)
    {
        this->emitSignal("valueChanged", this->value());
    }

private:
    void updateRange(void)
    {
        auto oldValue = this->value();
        _slider->setMinimum(_minimum/_stepSize);
        _slider->setMaximum(_maximum/_stepSize);
        this->setValue(oldValue);
    }

    double _minimum;
    double _maximum;
    double _stepSize;
    QSlider *_slider;
};

static Pothos::BlockRegistry registerAdd(
    "/widgets/slider", &Slider::make);

#include "Slider.moc"
