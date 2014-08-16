// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <qwt_slider.h>

/***********************************************************************
 * |PothosDoc QWT Slider
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
 * |param scalePosition[Scale Position] The slider scale markers.
 * |default "NoScale"
 * |option [None] "NoScale"
 * |option [Leading] "LeadingScale"
 * |option [Trailing] "TrailingScale"
 * |preview disable
 *
 * |param value The initial value of this slider.
 * |default 0.0
 * |widget DoubleSpinBox()
 *
 * |param lowerBound[Lower Bound] The minimum integer value of this slider.
 * |default 1.0
 * |widget DoubleSpinBox()
 *
 * |param upperBound[Upper Bound] The maximum integer value of this slider.
 * |default +1.0
 * |widget DoubleSpinBox()
 *
 * |param stepSize[Step Size] The delta inbetween discrete values in this slider.
 * |default 0.001
 * |widget DoubleSpinBox()
 *
 * |mode graphWidget
 * |factory /widgets/qwt_slider()
 * |setter setOrientation(orientation)
 * |setter setScalePosition(scalePosition)
 * |setter setLowerBound(lowerBound)
 * |setter setUpperBound(upperBound)
 * |setter setStepSize(stepSize)
 **********************************************************************/
class QwtSliderBlock : public QwtSlider, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new QwtSliderBlock();
    }

    QwtSliderBlock(void):
        QwtSlider(nullptr)
    {
        this->setHandleSize(QSize(14, 14));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, setLowerBound));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, setUpperBound));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, setStepSize));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, setOrientation));
        this->registerCall(this, POTHOS_FCN_TUPLE(QwtSliderBlock, setScalePosition));
        this->registerSignal("valueChanged");
        connect(this, SIGNAL(valueChanged(const double)), this, SLOT(handleValueChanged(const double)));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void setOrientation(const QString &orientation)
    {
        if ((orientation == "Horizontal")) QwtSlider::setOrientation(Qt::Horizontal);
        if ((orientation == "Vertical")) QwtSlider::setOrientation(Qt::Vertical);
    }

    void setScalePosition(const QString &scale)
    {
        if ((scale == "NoScale")) QwtSlider::setScalePosition(QwtSlider::NoScale);
        if ((scale == "LeadingScale")) QwtSlider::setScalePosition(QwtSlider::LeadingScale);
        if ((scale == "TrailingScale")) QwtSlider::setScalePosition(QwtSlider::TrailingScale);
    }

    void setStepSize(const double step)
    {
        this->setScaleStepSize(step);
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

private:
    QwtSlider *_slider;
};

static Pothos::BlockRegistry registerAdd(
    "/widgets/qwt_slider", &QwtSliderBlock::make);

#include "QwtSlider.moc"
