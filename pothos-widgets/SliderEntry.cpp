// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <QVBoxLayout>
#include <QWidget>

/***********************************************************************
 * |PothosDoc Slider Entry
 *
 * A combined Numeric Entry + Slider widget.
 *
 * |category /Widgets
 * |keywords numeric entry spinbox slider
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
 * |widget SpinBox(minimum=0, maximum=10)
 * |preview disable
 *
 * |mode graphWidget
 * |factory /widgets/slider_entry()
 * |setter setTitle(title)
 * |setter setValue(value)
 * |setter setMinimum(minimum)
 * |setter setMaximum(maximum)
 * |setter setSingleStep(step)
 * |setter setDecimals(precision)
 **********************************************************************/
class SliderEntry : public QWidget, public Pothos::Topology
{
    Q_OBJECT
public:

    static Topology *make(void)
    {
        return new SliderEntry();
    }

    SliderEntry(void)
    {
        auto env = Pothos::ProxyEnvironment::make("managed");
        auto registry = env->findProxy("Pothos/BlockRegistry");
        _slider = registry.callProxy("/widgets/slider", "Horizontal");
        _entry = registry.callProxy("/widgets/numeric_entry");

        auto layout = new QVBoxLayout(this);
        layout->setContentsMargins(QMargins());
        layout->setSpacing(0);
        layout->addWidget(_entry.call<QWidget *>("widget"), 0, Qt::AlignHCenter);
        layout->addWidget(_slider.call<QWidget *>("widget"));

        this->registerCall(this, POTHOS_FCN_TUPLE(SliderEntry, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(SliderEntry, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(SliderEntry, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(SliderEntry, setMinimum));
        this->registerCall(this, POTHOS_FCN_TUPLE(SliderEntry, setMaximum));
        this->registerCall(this, POTHOS_FCN_TUPLE(SliderEntry, setDecimals));
        this->registerCall(this, POTHOS_FCN_TUPLE(SliderEntry, setSingleStep));

        Topology::connect(this, "setTitle", _entry, "setTitle");

        Topology::connect(this, "setValue", _slider, "setValue");
        Topology::connect(this, "setValue", _entry, "setValue");
/*
        Topology::connect(this, "setValue", _slider, "setValue");
        Topology::connect(this, "setValue", _entry, "setValue");

*/
        Topology::connect(_slider, "valueChanged", _entry, "setValue");
        Topology::connect(_entry, "valueChanged", _slider, "setValue");
        Topology::connect(_slider, "valueChanged", this, "valueChanged");
        Topology::connect(_entry, "valueChanged", this, "valueChanged");
    }

    QWidget *widget(void)
    {
        return this;
    }

    void activate(void)
    {
    }

    void setTitle(const QString &title)
    {
    }

    void setValue(const double val)
    {
    }

    void setMinimum(const double min)
    {
    }

    void setMaximum(const double max)
    {
    }

    void setDecimals(const int prec)
    {
    }

    void setSingleStep(const double val)
    {
    }

private:
    Pothos::Proxy _slider;
    Pothos::Proxy _entry;
};

static Pothos::BlockRegistry registerAdd(
    "/widgets/slider_entry", &SliderEntry::make);

#include "SliderEntry.moc"
