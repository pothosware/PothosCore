// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "TimeDomainPlot.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <QHBoxLayout>

TimeDomainPlot::TimeDomainPlot(const Pothos::DType &dtype):
    _mainPlot(new QwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _displayRate(1.0),
    _sampleRate(1.0)
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(TimeDomainPlot, setSampleRate));
    this->setupInput(0, dtype);

    //layout
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);

    //setup plotter
    {
        //missing from qwt:
        qRegisterMetaType<QList<QwtLegendData>>("QList<QwtLegendData>");
        _mainPlot->setCanvasBackground(QBrush(QColor("white")));
        _mainPlot->insertLegend(new QwtLegend(_mainPlot));
    }

    //setup grid
    {
        _plotGrid->attach(_mainPlot);
        _plotGrid->setPen(QColor("#999999"), 0.5, Qt::DashLine);
    }
}

TimeDomainPlot::~TimeDomainPlot(void)
{
    return;
}

void TimeDomainPlot::setNumInputs(const size_t numInputs)
{
    for (size_t i = this->inputs().size(); i < numInputs; i++)
    {
        this->setupInput(i, this->input(0)->dtype());
    }
}

void TimeDomainPlot::setTitle(const QString &title)
{
    _mainPlot->setTitle(title);
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
