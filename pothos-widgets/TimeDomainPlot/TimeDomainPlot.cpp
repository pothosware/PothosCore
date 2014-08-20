// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "TimeDomainPlot.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <QHBoxLayout>

TimeDomainPlot::TimeDomainPlot(const Pothos::DType &dtype):
    _mainPlot(new QwtPlot(this)),
    _displayRate(1.0),
    _sampleRate(1.0)
{
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, setSampleRate));
    this->setupInput(0, dtype);

    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);
}

void TimeDomainPlot::setNumInputs(const size_t numInputs)
{
    for (size_t i = this->inputs().size(); i < numInputs; i++)
    {
        this->setupInput(i, this->input(0)->dtype());
    }
}

void TimeDomainPlot::setDisplayRate(const double displayRate)
{
    _displayRate = displayRate;
}

void TimeDomainPlot::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerTimeDomainPlot(
    "/widgets/time_domain_plot", &TimeDomainPlot::make);
